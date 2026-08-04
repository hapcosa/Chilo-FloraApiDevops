#include "../../include/repository/postgres_avistamiento_repository.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

constexpr const char* kSelectCols =
    "id, especie_id, reino, nombre_sugerido, descripcion, foto_key, "
    "geo_lat, geo_lng, precision_metros, observado_en, creado_por, estado, "
    "moderado_por, moderado_en, motivo_rechazo, grado_identificacion, "
    "created_at, updated_at";

std::optional<int> optInt(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return field.as<int>();
}

std::optional<double> optDouble(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return field.as<double>();
}

std::optional<std::string> optStr(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return std::string(field.c_str());
}

std::string quoteOptInt(pqxx::work& txn, const std::optional<int>& value) {
    return value ? txn.quote(*value) : "NULL";
}

std::string quoteOptDouble(pqxx::work& txn, const std::optional<double>& value) {
    return value ? txn.quote(*value) : "NULL";
}

std::string quoteOptString(pqxx::work& txn, const std::optional<std::string>& value) {
    return value ? txn.quote(*value) : "NULL";
}

} // namespace

PostgresAvistamientoRepository::PostgresAvistamientoRepository(
    std::shared_ptr<Database> database)
    : database(std::move(database)) {}

Avistamiento PostgresAvistamientoRepository::mapRowToAvistamiento(const pqxx::row& row) {
    Avistamiento avistamiento;
    avistamiento.setId(row["id"].as<int>());
    avistamiento.setEspecieId(optInt(row["especie_id"]));
    avistamiento.setReino(reinoFromString(row["reino"].c_str()));
    avistamiento.setNombreSugerido(optStr(row["nombre_sugerido"]));
    avistamiento.setDescripcion(optStr(row["descripcion"]));
    avistamiento.setFotoKey(row["foto_key"].c_str());
    avistamiento.setGeoLat(row["geo_lat"].as<double>());
    avistamiento.setGeoLng(row["geo_lng"].as<double>());
    avistamiento.setPrecisionMetros(optDouble(row["precision_metros"]));
    avistamiento.setObservadoEn(optStr(row["observado_en"]));
    avistamiento.setCreadoPor(optInt(row["creado_por"]));
    avistamiento.setEstado(avistamientoEstadoFromString(row["estado"].c_str()));
    avistamiento.setModeradoPor(optInt(row["moderado_por"]));
    avistamiento.setModeradoEn(optStr(row["moderado_en"]));
    avistamiento.setMotivoRechazo(optStr(row["motivo_rechazo"]));
    avistamiento.setGradoIdentificacion(
        gradoIdentificacionFromString(row["grado_identificacion"].c_str()));
    avistamiento.setCreatedAt(optStr(row["created_at"]));
    avistamiento.setUpdatedAt(optStr(row["updated_at"]));
    return avistamiento;
}

Avistamiento PostgresAvistamientoRepository::create(const Avistamiento& avistamiento) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const std::string sql =
            std::string("INSERT INTO avistamientos (")
            + "especie_id, reino, nombre_sugerido, descripcion, foto_key, "
            + "geo_lat, geo_lng, precision_metros, observado_en, creado_por"
            + ") VALUES ("
            + quoteOptInt(txn, avistamiento.getEspecieId()) + ", "
            + txn.quote(reinoToString(avistamiento.getReino())) + "::reino_enum, "
            + quoteOptString(txn, avistamiento.getNombreSugerido()) + ", "
            + quoteOptString(txn, avistamiento.getDescripcion()) + ", "
            + txn.quote(avistamiento.getFotoKey()) + ", "
            + txn.quote(avistamiento.getGeoLat()) + ", "
            + txn.quote(avistamiento.getGeoLng()) + ", "
            + quoteOptDouble(txn, avistamiento.getPrecisionMetros()) + ", "
            + (avistamiento.getObservadoEn()
                   ? txn.quote(*avistamiento.getObservadoEn())
                   : std::string("NOW()")) + ", "
            + quoteOptInt(txn, avistamiento.getCreadoPor())
            + ") RETURNING " + kSelectCols;

        pqxx::result result = txn.exec(sql);
        txn.commit();
        return mapRowToAvistamiento(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al crear avistamiento: " << error.what() << std::endl;
        throw;
    }
}

AvistamientoSearchResult PostgresAvistamientoRepository::find(
    const AvistamientoFilters& filters) {
    if (filters.limit < 1 || filters.limit > 200) {
        throw std::invalid_argument("limit debe estar en [1, 200]");
    }
    if (filters.offset < 0) {
        throw std::invalid_argument("offset debe ser >= 0");
    }

    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        std::string where = " WHERE 1=1";
        if (filters.estado) {
            where += " AND estado = "
                + txn.quote(avistamientoEstadoToString(*filters.estado))
                + "::avistamiento_estado_enum";
        }
        if (filters.reino) {
            where += " AND reino = " + txn.quote(reinoToString(*filters.reino))
                + "::reino_enum";
        }
        if (filters.especie_id) {
            where += " AND especie_id = " + txn.quote(*filters.especie_id);
        }
        if (filters.creado_por) {
            where += " AND creado_por = " + txn.quote(*filters.creado_por);
        }

        const auto countResult = txn.exec("SELECT COUNT(*) FROM avistamientos" + where);
        const int total = countResult[0][0].as<int>();

        const std::string dataSql =
            std::string("SELECT ") + kSelectCols + " FROM avistamientos"
            + where
            + " ORDER BY observado_en DESC"
            + " LIMIT " + std::to_string(filters.limit)
            + " OFFSET " + std::to_string(filters.offset);

        const auto rows = txn.exec(dataSql);

        AvistamientoSearchResult result;
        result.total = total;
        result.data.reserve(rows.size());
        for (const auto& row : rows) {
            result.data.push_back(mapRowToAvistamiento(row));
        }
        return result;
    } catch (const std::exception& error) {
        std::cerr << "Error al buscar avistamientos: " << error.what() << std::endl;
        throw;
    }
}

std::optional<Avistamiento> PostgresAvistamientoRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto result = txn.exec_params(
            std::string("SELECT ") + kSelectCols + " FROM avistamientos WHERE id = $1",
            id);

        if (result.empty()) return std::nullopt;
        return mapRowToAvistamiento(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al obtener avistamiento: " << error.what() << std::endl;
        throw;
    }
}

Avistamiento PostgresAvistamientoRepository::moderate(
    int id,
    const ModeracionAvistamiento& moderacion) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const std::string sql =
            std::string("UPDATE avistamientos SET ")
            + "estado = " + txn.quote(avistamientoEstadoToString(moderacion.estado))
            + "::avistamiento_estado_enum, "
            + "moderado_por = " + quoteOptInt(txn, moderacion.moderado_por) + ", "
            + "moderado_en = NOW(), "
            + "motivo_rechazo = " + quoteOptString(txn, moderacion.motivo_rechazo)
            + " WHERE id = " + txn.quote(id)
            + " RETURNING " + kSelectCols;

        const auto result = txn.exec(sql);
        if (result.empty()) {
            throw std::out_of_range("avistamiento no encontrado");
        }

        txn.commit();
        return mapRowToAvistamiento(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al moderar avistamiento: " << error.what() << std::endl;
        throw;
    }
}
