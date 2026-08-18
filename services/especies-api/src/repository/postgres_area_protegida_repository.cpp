#include "../../include/repository/postgres_area_protegida_repository.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

constexpr const char* kSelectCols =
    "id, nombre, tipo, descripcion, administrador, accesos, sitio_web, "
    "centro_lat, centro_lng, min_lat, min_lng, max_lat, max_lng, "
    "geometria, superficie_ha, fuente, verificado, created_at, updated_at";

std::optional<std::string> optStr(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return std::string(field.c_str());
}

std::optional<double> optDouble(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return field.as<double>();
}

} // namespace

PostgresAreaProtegidaRepository::PostgresAreaProtegidaRepository(
    std::shared_ptr<Database> database)
    : database(std::move(database)) {}

AreaProtegida PostgresAreaProtegidaRepository::mapRowToArea(const pqxx::row& row) {
    AreaProtegida area;
    area.id = row["id"].as<int>();
    area.nombre = row["nombre"].c_str();
    area.tipo = areaProtegidaTipoFromString(row["tipo"].c_str());
    area.descripcion = optStr(row["descripcion"]);
    area.administrador = optStr(row["administrador"]);
    area.accesos = optStr(row["accesos"]);
    area.sitio_web = optStr(row["sitio_web"]);
    area.centro_lat = row["centro_lat"].as<double>();
    area.centro_lng = row["centro_lng"].as<double>();
    area.min_lat = row["min_lat"].as<double>();
    area.min_lng = row["min_lng"].as<double>();
    area.max_lat = row["max_lat"].as<double>();
    area.max_lng = row["max_lng"].as<double>();
    if (!row["geometria"].is_null()) {
        // Texto guardado por curaduría: si no parsea, la ficha se sirve sin
        // polígono en vez de tumbar el listado entero.
        area.geometria = nlohmann::json::parse(row["geometria"].c_str(), nullptr, false);
        if (area.geometria->is_discarded()) {
            area.geometria = std::nullopt;
        }
    }
    area.superficie_ha = optDouble(row["superficie_ha"]);
    area.fuente = optStr(row["fuente"]);
    area.verificado = row["verificado"].as<bool>();
    area.created_at = optStr(row["created_at"]);
    area.updated_at = optStr(row["updated_at"]);
    return area;
}

std::vector<AreaProtegida> PostgresAreaProtegidaRepository::find(
    const AreaProtegidaFilters& filters) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        std::string sql = std::string("SELECT ") + kSelectCols
                        + " FROM areas_protegidas WHERE 1 = 1";

        if (filters.tipo) {
            sql += " AND tipo = " + txn.quote(areaProtegidaTipoToString(*filters.tipo))
                 + "::area_protegida_tipo_enum";
        }
        // Solapamiento de rectángulos, no contención: un parque más grande que
        // la pantalla igual tiene que salir.
        if (filters.min_lat && filters.min_lng && filters.max_lat && filters.max_lng) {
            sql += " AND min_lat <= " + txn.quote(*filters.max_lat)
                 + " AND max_lat >= " + txn.quote(*filters.min_lat)
                 + " AND min_lng <= " + txn.quote(*filters.max_lng)
                 + " AND max_lng >= " + txn.quote(*filters.min_lng);
        }

        sql += " ORDER BY nombre";

        pqxx::result result = txn.exec(sql);
        txn.commit();

        std::vector<AreaProtegida> areas;
        areas.reserve(result.size());
        for (const auto& row : result) {
            areas.push_back(mapRowToArea(row));
        }
        return areas;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar áreas protegidas: " << error.what() << std::endl;
        throw;
    }
}

std::optional<AreaProtegida> PostgresAreaProtegidaRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec(
            std::string("SELECT ") + kSelectCols
            + " FROM areas_protegidas WHERE id = " + txn.quote(id));
        txn.commit();

        if (result.empty()) {
            return std::nullopt;
        }
        return mapRowToArea(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al buscar el área protegida: " << error.what() << std::endl;
        throw;
    }
}

// Qué se ha visto dentro del área. Solo encuentros públicos y aprobados, y solo
// los que tienen especie asignada: un "no identificado" no le dice nada a quien
// está eligiendo a qué parque ir.
//
// El filtro es por bounding box, no por el polígono: sin PostGIS no hay
// `ST_Contains`, y el bbox incluye de más en una costa tan recortada como la de
// Chiloé. Es la aproximación que documenta la migración 0012; se puede afinar
// el día que exista `geometria` cargada y valga la pena filtrar en el cliente.
std::vector<EspecieEnArea> PostgresAreaProtegidaRepository::especiesEnArea(int areaId,
                                                                          int limit) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const std::string sql =
            "SELECT e.id AS especie_id, e.nombre_comun, e.nombre_cientifico, e.reino,"
            " COUNT(*) AS avistamientos, MAX(a.observado_en) AS ultimo"
            " FROM areas_protegidas ap"
            " JOIN avistamientos a"
            "   ON a.geo_lat BETWEEN ap.min_lat AND ap.max_lat"
            "  AND a.geo_lng BETWEEN ap.min_lng AND ap.max_lng"
            " JOIN especies e ON e.id = a.especie_id"
            " WHERE ap.id = " + txn.quote(areaId)
            + " AND a.estado = 'aprobado' AND a.visibilidad = 'publico'"
            + " GROUP BY e.id, e.nombre_comun, e.nombre_cientifico, e.reino"
            + " ORDER BY avistamientos DESC, e.nombre_comun"
            + " LIMIT " + txn.quote(limit);

        pqxx::result result = txn.exec(sql);
        txn.commit();

        std::vector<EspecieEnArea> especies;
        especies.reserve(result.size());
        for (const auto& row : result) {
            EspecieEnArea especie;
            especie.especie_id = row["especie_id"].as<int>();
            especie.nombre_comun = row["nombre_comun"].c_str();
            especie.nombre_cientifico = row["nombre_cientifico"].c_str();
            especie.reino = reinoFromString(row["reino"].c_str());
            especie.avistamientos = row["avistamientos"].as<int>();
            especie.ultimo_avistamiento = optStr(row["ultimo"]);
            especies.push_back(especie);
        }
        return especies;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar especies del área: " << error.what() << std::endl;
        throw;
    }
}
