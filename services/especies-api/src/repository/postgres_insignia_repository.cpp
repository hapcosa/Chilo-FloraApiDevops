#include "../../include/repository/postgres_insignia_repository.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <utility>

#include "../../include/utils/timestamps.hpp"

namespace {

constexpr const char* kSelectCols =
    "id, codigo, nombre, descripcion, criterio, tipo, metrica, umbral";

// Literal de array de Postgres, `{1,2,3}`, para pasar la lista entera como un
// solo parámetro en lugar de armar N placeholders. Los ids ya vienen parseados
// como enteros, así que no hay nada que escapar.
std::string arrayLiteral(const std::vector<int>& ids) {
    std::string literal = "{";
    for (std::size_t i = 0; i < ids.size(); ++i) {
        if (i > 0) literal += ',';
        literal += std::to_string(ids[i]);
    }
    return literal + "}";
}

std::optional<std::string> optStr(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return std::string(field.c_str());
}

std::optional<int> optInt(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return field.as<int>();
}

std::string quoteOptString(pqxx::work& txn, const std::optional<std::string>& value) {
    return value ? txn.quote(*value) : "NULL";
}

} // namespace

PostgresInsigniaRepository::PostgresInsigniaRepository(
    std::shared_ptr<Database> database)
    : database(std::move(database)) {}

Insignia PostgresInsigniaRepository::mapRowToInsignia(const pqxx::row& row) {
    Insignia insignia;
    insignia.setId(row["id"].as<int>());
    insignia.setCodigo(row["codigo"].c_str());
    insignia.setNombre(row["nombre"].c_str());
    insignia.setDescripcion(row["descripcion"].c_str());
    insignia.setCriterio(row["criterio"].c_str());
    insignia.setTipo(insigniaTipoFromString(row["tipo"].c_str()));

    const auto metrica = optStr(row["metrica"]);
    insignia.setMetrica(metrica ? std::optional<InsigniaMetrica>(
                                      insigniaMetricaFromString(*metrica))
                                : std::nullopt);
    insignia.setUmbral(optInt(row["umbral"]));
    return insignia;
}

std::vector<Insignia> PostgresInsigniaRepository::findAll() {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        // Las automáticas primero y por umbral: es el orden en que se ganan y
        // el que el cliente pinta sin tener que reordenar.
        const auto rows = txn.exec(
            std::string("SELECT ") + kSelectCols
            + " FROM insignias ORDER BY tipo, metrica, umbral, codigo");

        std::vector<Insignia> insignias;
        insignias.reserve(rows.size());
        for (const auto& row : rows) {
            insignias.push_back(mapRowToInsignia(row));
        }
        return insignias;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar insignias: " << error.what() << std::endl;
        throw;
    }
}

std::optional<Insignia> PostgresInsigniaRepository::findByCodigo(
    const std::string& codigo) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
                + " FROM insignias WHERE codigo = $1",
            codigo);

        if (result.empty()) return std::nullopt;
        return mapRowToInsignia(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al obtener insignia: " << error.what() << std::endl;
        throw;
    }
}

std::vector<InsigniaOtorgada> PostgresInsigniaRepository::findByUsuario(
    int usuarioId) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto rows = txn.exec_params(
            "SELECT i.id, i.codigo, i.nombre, i.descripcion, i.criterio,"
            " i.tipo, i.metrica, i.umbral,"
            " ui.otorgada_en, ui.otorgada_por, ui.motivo"
            " FROM usuario_insignias ui"
            " JOIN insignias i ON i.id = ui.insignia_id"
            " WHERE ui.usuario_id = $1"
            // Las más recientes primero: el perfil muestra lo último ganado.
            " ORDER BY ui.otorgada_en DESC, i.codigo",
            usuarioId);

        std::vector<InsigniaOtorgada> otorgadas;
        otorgadas.reserve(rows.size());
        for (const auto& row : rows) {
            InsigniaOtorgada otorgada;
            otorgada.setInsignia(mapRowToInsignia(row));
            otorgada.setOtorgadaEn(
                utils::toIso8601Opt(optStr(row["otorgada_en"])).value_or(""));
            otorgada.setOtorgadaPor(optInt(row["otorgada_por"]));
            otorgada.setMotivo(optStr(row["motivo"]));
            otorgadas.push_back(std::move(otorgada));
        }
        return otorgadas;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar insignias del usuario: " << error.what()
                  << std::endl;
        throw;
    }
}

std::map<int, std::vector<InsigniaOtorgada>>
PostgresInsigniaRepository::findByUsuarios(const std::vector<int>& usuarioIds) {
    // Toda clave pedida aparece en el resultado, aunque esa persona no tenga
    // ninguna insignia: así el cliente no tiene que cruzar contra su pedido.
    std::map<int, std::vector<InsigniaOtorgada>> porUsuario;
    for (const int id : usuarioIds) {
        porUsuario.emplace(id, std::vector<InsigniaOtorgada>{});
    }
    if (usuarioIds.empty()) return porUsuario;

    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto rows = txn.exec_params(
            "SELECT ui.usuario_id,"
            " i.id, i.codigo, i.nombre, i.descripcion, i.criterio,"
            " i.tipo, i.metrica, i.umbral,"
            " ui.otorgada_en, ui.otorgada_por, ui.motivo"
            " FROM usuario_insignias ui"
            " JOIN insignias i ON i.id = ui.insignia_id"
            " WHERE ui.usuario_id = ANY($1::int[])"
            // Mismo orden que findByUsuario, para que una fila del lote se vea
            // igual que la misma fila pedida sola.
            " ORDER BY ui.usuario_id, ui.otorgada_en DESC, i.codigo",
            arrayLiteral(usuarioIds));

        for (const auto& row : rows) {
            InsigniaOtorgada otorgada;
            otorgada.setInsignia(mapRowToInsignia(row));
            otorgada.setOtorgadaEn(
                utils::toIso8601Opt(optStr(row["otorgada_en"])).value_or(""));
            otorgada.setOtorgadaPor(optInt(row["otorgada_por"]));
            otorgada.setMotivo(optStr(row["motivo"]));
            porUsuario[row["usuario_id"].as<int>()].push_back(std::move(otorgada));
        }
        return porUsuario;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar insignias de varios usuarios: "
                  << error.what() << std::endl;
        throw;
    }
}

bool PostgresInsigniaRepository::otorgar(
    int usuarioId, int insigniaId, int otorgadaPor,
    const std::optional<std::string>& motivo) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        // `motivo` va citado a mano porque es opcional: exec_params no
        // distingue un std::optional vacío de la cadena vacía.
        const auto result = txn.exec(
            "INSERT INTO usuario_insignias"
            " (usuario_id, insignia_id, otorgada_por, motivo) VALUES ("
            + txn.quote(usuarioId) + ", " + txn.quote(insigniaId) + ", "
            + txn.quote(otorgadaPor) + ", " + quoteOptString(txn, motivo) + ")"
            " ON CONFLICT (usuario_id, insignia_id) DO NOTHING"
            " RETURNING usuario_id");

        txn.commit();
        return !result.empty();
    } catch (const std::exception& error) {
        std::cerr << "Error al otorgar insignia: " << error.what() << std::endl;
        throw;
    }
}

bool PostgresInsigniaRepository::revocar(int usuarioId, int insigniaId) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto result = txn.exec_params(
            "DELETE FROM usuario_insignias"
            " WHERE usuario_id = $1 AND insignia_id = $2",
            usuarioId, insigniaId);

        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception& error) {
        std::cerr << "Error al revocar insignia: " << error.what() << std::endl;
        throw;
    }
}

int PostgresInsigniaRepository::recalcularAutomaticas() {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Una sola sentencia para todo el catálogo automático: las métricas se
        // calculan por usuario y se cruzan con `insignias` por (metrica,
        // umbral). Añadir una insignia nueva es una fila en el catálogo, no
        // una rama aquí.
        //
        // Solo cuentan los encuentros aprobados: si contaran los pendientes,
        // subir basura y esperar el rechazo daría insignias igual.
        const auto result = txn.exec(
            "WITH aprobados AS ("
            "  SELECT creado_por AS usuario_id, id, especie_id, reino"
            "    FROM avistamientos"
            "   WHERE estado = 'aprobado' AND creado_por IS NOT NULL"
            "),"
            "metricas AS ("
            "  SELECT usuario_id, 'encuentros'::insignia_metrica_enum AS metrica,"
            "         COUNT(*) AS valor"
            "    FROM aprobados GROUP BY usuario_id"
            "  UNION ALL"
            "  SELECT usuario_id, 'especies_distintas'::insignia_metrica_enum,"
            "         COUNT(DISTINCT especie_id)"
            "    FROM aprobados WHERE especie_id IS NOT NULL GROUP BY usuario_id"
            "  UNION ALL"
            "  SELECT usuario_id, 'reinos'::insignia_metrica_enum,"
            "         COUNT(DISTINCT reino)"
            "    FROM aprobados GROUP BY usuario_id"
            "  UNION ALL"
            // Identificado por otros: cuenta encuentros propios con al menos
            // una identificación vigente ajena. Autoidentificarse no suma.
            "  SELECT a.usuario_id, 'identificado_por_otros'::insignia_metrica_enum,"
            "         COUNT(DISTINCT a.id)"
            "    FROM aprobados a"
            "    JOIN avistamiento_identificaciones ai"
            "      ON ai.avistamiento_id = a.id"
            "     AND NOT ai.retirada"
            "     AND ai.usuario_id <> a.usuario_id"
            "   GROUP BY a.usuario_id"
            ")"
            "INSERT INTO usuario_insignias (usuario_id, insignia_id, motivo)"
            " SELECT m.usuario_id, i.id, i.criterio"
            "   FROM metricas m"
            "   JOIN insignias i"
            "     ON i.tipo = 'automatica'"
            "    AND i.metrica = m.metrica"
            "    AND m.valor >= i.umbral"
            " ON CONFLICT (usuario_id, insignia_id) DO NOTHING");

        txn.commit();
        return static_cast<int>(result.affected_rows());
    } catch (const std::exception& error) {
        std::cerr << "Error al recalcular insignias: " << error.what() << std::endl;
        throw;
    }
}
