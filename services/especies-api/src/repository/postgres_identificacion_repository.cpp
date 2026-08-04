#include "../../include/repository/postgres_identificacion_repository.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

constexpr const char* kSelectCols =
    "id, avistamiento_id, usuario_id, especie_id, comentario, decisiva, "
    "retirada, created_at, updated_at";

std::optional<std::string> optStr(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return std::string(field.c_str());
}

} // namespace

PostgresIdentificacionRepository::PostgresIdentificacionRepository(
    std::shared_ptr<Database> database)
    : database(std::move(database)) {}

Identificacion PostgresIdentificacionRepository::mapRowToIdentificacion(
    const pqxx::row& row) {
    Identificacion identificacion;
    identificacion.setId(row["id"].as<int>());
    identificacion.setAvistamientoId(row["avistamiento_id"].as<int>());
    identificacion.setUsuarioId(row["usuario_id"].as<int>());
    identificacion.setEspecieId(row["especie_id"].as<int>());
    identificacion.setComentario(optStr(row["comentario"]));
    identificacion.setDecisiva(row["decisiva"].as<bool>());
    identificacion.setRetirada(row["retirada"].as<bool>());
    identificacion.setCreatedAt(optStr(row["created_at"]));
    identificacion.setUpdatedAt(optStr(row["updated_at"]));
    return identificacion;
}

void PostgresIdentificacionRepository::recalcularGrado(
    pqxx::work& txn, int avistamientoId, const ReglaGrado& regla) {
    const auto rows = txn.exec_params(
        std::string("SELECT ") + kSelectCols
            + " FROM avistamiento_identificaciones"
              " WHERE avistamiento_id = $1 AND NOT retirada"
              " ORDER BY id",
        avistamientoId);

    std::vector<Identificacion> vigentes;
    vigentes.reserve(rows.size());
    for (const auto& row : rows) {
        vigentes.push_back(mapRowToIdentificacion(row));
    }

    const auto resultado = regla(vigentes);

    if (resultado.especie_id) {
        txn.exec_params(
            "UPDATE avistamientos"
            " SET grado_identificacion = $1::grado_identificacion_enum, especie_id = $2"
            " WHERE id = $3",
            gradoIdentificacionToString(resultado.grado),
            *resultado.especie_id,
            avistamientoId);
    } else {
        // `especie_id` no se limpia al bajar de grado: puede haberla puesto un
        // moderador a mano antes de que existiera este flujo, y borrarla sería
        // destruir un dato que estas identificaciones no contradicen —solo no
        // lo respaldan todavía.
        txn.exec_params(
            "UPDATE avistamientos"
            " SET grado_identificacion = $1::grado_identificacion_enum"
            " WHERE id = $2",
            gradoIdentificacionToString(resultado.grado),
            avistamientoId);
    }
}

std::vector<Identificacion> PostgresIdentificacionRepository::findByAvistamiento(
    int avistamientoId) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto rows = txn.exec_params(
            std::string("SELECT ") + kSelectCols
                + " FROM avistamiento_identificaciones"
                  " WHERE avistamiento_id = $1 ORDER BY created_at, id",
            avistamientoId);

        std::vector<Identificacion> identificaciones;
        identificaciones.reserve(rows.size());
        for (const auto& row : rows) {
            identificaciones.push_back(mapRowToIdentificacion(row));
        }
        return identificaciones;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar identificaciones: " << error.what() << std::endl;
        throw;
    }
}

std::optional<Identificacion> PostgresIdentificacionRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
                + " FROM avistamiento_identificaciones WHERE id = $1",
            id);

        if (result.empty()) return std::nullopt;
        return mapRowToIdentificacion(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al obtener identificación: " << error.what() << std::endl;
        throw;
    }
}

Identificacion PostgresIdentificacionRepository::create(
    const Identificacion& identificacion, const ReglaGrado& regla) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Bloquea la fila del avistamiento antes de escribir: dos personas
        // identificando a la vez se serializan aquí y cada recálculo ve el
        // conteo completo. Sin esto el último UPDATE del grado podría venir de
        // la transacción que contó de menos.
        const auto avistamiento = txn.exec_params(
            "SELECT id FROM avistamientos WHERE id = $1 FOR UPDATE",
            identificacion.getAvistamientoId());
        if (avistamiento.empty()) {
            throw std::out_of_range("avistamiento no encontrado");
        }

        const auto result = txn.exec_params(
            std::string("INSERT INTO avistamiento_identificaciones"
                        " (avistamiento_id, usuario_id, especie_id, comentario, decisiva)"
                        " VALUES ($1, $2, $3, $4, $5) RETURNING ") + kSelectCols,
            identificacion.getAvistamientoId(),
            identificacion.getUsuarioId(),
            identificacion.getEspecieId(),
            identificacion.getComentario(),
            identificacion.esDecisiva());

        recalcularGrado(txn, identificacion.getAvistamientoId(), regla);

        txn.commit();
        return mapRowToIdentificacion(result[0]);
    } catch (const pqxx::unique_violation&) {
        // Índice único parcial: ya tiene una vigente en este avistamiento.
        throw std::invalid_argument(
            "ya identificaste este avistamiento; retira la anterior para cambiarla");
    } catch (const pqxx::foreign_key_violation&) {
        throw std::out_of_range("especie no encontrada");
    } catch (const std::exception& error) {
        std::cerr << "Error al crear identificación: " << error.what() << std::endl;
        throw;
    }
}

std::optional<Identificacion> PostgresIdentificacionRepository::retirar(
    int id, const ReglaGrado& regla) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const auto avistamiento = txn.exec_params(
            "SELECT a.id FROM avistamientos a"
            " JOIN avistamiento_identificaciones i ON i.avistamiento_id = a.id"
            " WHERE i.id = $1 FOR UPDATE OF a",
            id);
        if (avistamiento.empty()) return std::nullopt;

        const auto result = txn.exec_params(
            std::string("UPDATE avistamiento_identificaciones SET retirada = true"
                        " WHERE id = $1 AND NOT retirada RETURNING ") + kSelectCols,
            id);
        if (result.empty()) return std::nullopt;

        const auto retirada = mapRowToIdentificacion(result[0]);
        recalcularGrado(txn, retirada.getAvistamientoId(), regla);

        txn.commit();
        return retirada;
    } catch (const std::exception& error) {
        std::cerr << "Error al retirar identificación: " << error.what() << std::endl;
        throw;
    }
}
