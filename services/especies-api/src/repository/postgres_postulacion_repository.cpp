#include "../../include/repository/postgres_postulacion_repository.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "../../include/utils/timestamps.hpp"

namespace {

constexpr const char* kSelectCols =
    "id, usuario_id, categoria_id, texto, estado, revisado_por, revisado_en, "
    "motivo, created_at, updated_at";

std::optional<std::string> optStr(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return std::string(field.c_str());
}

std::optional<int> optInt(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return field.as<int>();
}

} // namespace

PostgresPostulacionRepository::PostgresPostulacionRepository(
    std::shared_ptr<Database> database)
    : database(std::move(database)) {}

PostulacionCurador PostgresPostulacionRepository::mapRowToPostulacion(
    const pqxx::row& row) {
    PostulacionCurador postulacion;
    postulacion.setId(row["id"].as<int>());
    postulacion.setUsuarioId(row["usuario_id"].as<int>());
    postulacion.setCategoriaId(row["categoria_id"].as<int>());
    postulacion.setTexto(row["texto"].c_str());
    postulacion.setEstado(postulacionEstadoFromString(row["estado"].c_str()));
    postulacion.setRevisadoPor(optInt(row["revisado_por"]));
    postulacion.setRevisadoEn(utils::toIso8601Opt(optStr(row["revisado_en"])));
    postulacion.setMotivo(optStr(row["motivo"]));
    postulacion.setCreatedAt(utils::toIso8601Opt(optStr(row["created_at"])));
    postulacion.setUpdatedAt(utils::toIso8601Opt(optStr(row["updated_at"])));
    return postulacion;
}

std::vector<PostulacionCurador> PostgresPostulacionRepository::findAll(
    std::optional<PostulacionEstado> estado) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        std::string sql = std::string("SELECT ") + kSelectCols
            + " FROM postulaciones_curador";
        if (estado) {
            sql += " WHERE estado = "
                + txn.quote(postulacionEstadoToString(*estado))
                + "::postulacion_estado_enum";
        }
        // Más antiguas primero: la bandeja se atiende por orden de llegada.
        sql += " ORDER BY created_at";

        const auto rows = txn.exec(sql);

        std::vector<PostulacionCurador> postulaciones;
        postulaciones.reserve(rows.size());
        for (const auto& row : rows) {
            postulaciones.push_back(mapRowToPostulacion(row));
        }
        return postulaciones;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar postulaciones: " << error.what() << std::endl;
        throw;
    }
}

std::vector<PostulacionCurador> PostgresPostulacionRepository::findByUsuario(
    int usuarioId) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto rows = txn.exec_params(
            std::string("SELECT ") + kSelectCols
                + " FROM postulaciones_curador WHERE usuario_id = $1"
                  " ORDER BY created_at DESC",
            usuarioId);

        std::vector<PostulacionCurador> postulaciones;
        postulaciones.reserve(rows.size());
        for (const auto& row : rows) {
            postulaciones.push_back(mapRowToPostulacion(row));
        }
        return postulaciones;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar postulaciones del usuario: "
                  << error.what() << std::endl;
        throw;
    }
}

std::optional<PostulacionCurador> PostgresPostulacionRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
                + " FROM postulaciones_curador WHERE id = $1",
            id);

        if (result.empty()) return std::nullopt;
        return mapRowToPostulacion(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al obtener postulación: " << error.what() << std::endl;
        throw;
    }
}

PostulacionCurador PostgresPostulacionRepository::create(
    const PostulacionCurador& postulacion) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const auto result = txn.exec_params(
            std::string("INSERT INTO postulaciones_curador"
                        " (usuario_id, categoria_id, texto)"
                        " VALUES ($1, $2, $3) RETURNING ") + kSelectCols,
            postulacion.getUsuarioId(),
            postulacion.getCategoriaId(),
            postulacion.getTexto());

        txn.commit();
        return mapRowToPostulacion(result[0]);
    } catch (const pqxx::unique_violation&) {
        // Índice único parcial: ya hay una pendiente para ese par.
        throw std::invalid_argument(
            "ya tienes una postulación pendiente para esta categoría");
    } catch (const pqxx::foreign_key_violation&) {
        throw std::out_of_range("categoría no encontrada");
    } catch (const std::exception& error) {
        std::cerr << "Error al crear postulación: " << error.what() << std::endl;
        throw;
    }
}

PostulacionCurador PostgresPostulacionRepository::aprobar(int id, int revisadoPor) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // El WHERE por estado hace de guard contra la doble aprobación: si dos
        // admins resuelven a la vez, la segunda no encuentra fila que tocar.
        const auto result = txn.exec_params(
            std::string("UPDATE postulaciones_curador SET"
                        " estado = 'aprobada', revisado_por = $1, revisado_en = NOW()"
                        " WHERE id = $2 AND estado = 'pendiente' RETURNING ")
                + kSelectCols,
            revisadoPor, id);

        if (result.empty()) {
            throw std::invalid_argument("la postulación ya fue resuelta");
        }

        const auto postulacion = mapRowToPostulacion(result[0]);

        // Misma transacción que el UPDATE: aprobar sin asignar dejaría al
        // curador sin permisos y con la solicitud cerrada. ON CONFLICT porque
        // un admin puede haberlo asignado ya a mano.
        txn.exec_params(
            "INSERT INTO moderador_categorias (usuario_id, categoria_id, asignado_por)"
            " VALUES ($1, $2, $3)"
            " ON CONFLICT (usuario_id, categoria_id) DO NOTHING",
            postulacion.getUsuarioId(),
            postulacion.getCategoriaId(),
            revisadoPor);

        txn.commit();
        return postulacion;
    } catch (const std::exception& error) {
        std::cerr << "Error al aprobar postulación: " << error.what() << std::endl;
        throw;
    }
}

PostulacionCurador PostgresPostulacionRepository::rechazar(
    int id, int revisadoPor, const std::string& motivo) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const auto result = txn.exec_params(
            std::string("UPDATE postulaciones_curador SET"
                        " estado = 'rechazada', revisado_por = $1,"
                        " revisado_en = NOW(), motivo = $2"
                        " WHERE id = $3 AND estado = 'pendiente' RETURNING ")
                + kSelectCols,
            revisadoPor, motivo, id);

        if (result.empty()) {
            throw std::invalid_argument("la postulación ya fue resuelta");
        }

        txn.commit();
        return mapRowToPostulacion(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al rechazar postulación: " << error.what() << std::endl;
        throw;
    }
}
