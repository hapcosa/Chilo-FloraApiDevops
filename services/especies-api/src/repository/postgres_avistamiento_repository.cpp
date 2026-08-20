#include "../../include/repository/postgres_avistamiento_repository.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "../../include/utils/conservacion.hpp"
#include "../../include/utils/timestamps.hpp"

namespace {

constexpr const char* kSelectCols =
    "id, especie_id, reino, nombre_sugerido, descripcion, foto_key, "
    "geo_lat, geo_lng, precision_metros, precision_declarada, observado_en, "
    "creado_por, estado, "
    "visibilidad, moderado_por, moderado_en, motivo_rechazo, "
    "grado_identificacion, created_at, updated_at";

// Conteo de identificaciones vigentes, resuelto en la misma consulta que trae
// las filas: el feed lo necesita por tarjeta y una petición por fila no escala.
// Se apoya en idx_avistamiento_identificaciones_avistamiento, que ya es parcial
// sobre `NOT retirada` (migración 0007).
constexpr const char* kCountCol =
    ", (SELECT COUNT(*) FROM avistamiento_identificaciones ai"
    " WHERE ai.avistamiento_id = a.id AND NOT ai.retirada)"
    " AS identificaciones_count";

// ¿La especie del encuentro está en categoría de riesgo? Subconsulta y no
// LEFT JOIN para no tener que calificar cada columna del SELECT: el join haría
// ambiguo `id`, que también existe en `especies`. COALESCE porque un encuentro
// sin especie asignada deja el escalar en NULL, y sin especie no hay nada que
// proteger. Mismo patrón que el mapa, generado desde la misma lista de tokens.
std::string sensibleCol(pqxx::work& txn) {
    return ", COALESCE((SELECT e.estado_conservacion ~* "
           + txn.quote(utils::patronSqlEstadoSensible())
           + " FROM especies e WHERE e.id = a.especie_id), false) AS especie_sensible";
}

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

Avistamiento PostgresAvistamientoRepository::mapRowToAvistamiento(const pqxx::row& row,
                                                                 bool withCount) {
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
    avistamiento.setPrecisionDeclarada(
        precisionDeclaradaFromString(row["precision_declarada"].c_str()));
    avistamiento.setObservadoEn(utils::toIso8601Opt(optStr(row["observado_en"])));
    avistamiento.setCreadoPor(optInt(row["creado_por"]));
    avistamiento.setEstado(avistamientoEstadoFromString(row["estado"].c_str()));
    avistamiento.setVisibilidad(
        avistamientoVisibilidadFromString(row["visibilidad"].c_str()));
    avistamiento.setModeradoPor(optInt(row["moderado_por"]));
    avistamiento.setModeradoEn(utils::toIso8601Opt(optStr(row["moderado_en"])));
    avistamiento.setMotivoRechazo(optStr(row["motivo_rechazo"]));
    avistamiento.setGradoIdentificacion(
        gradoIdentificacionFromString(row["grado_identificacion"].c_str()));
    avistamiento.setCreatedAt(utils::toIso8601Opt(optStr(row["created_at"])));
    avistamiento.setUpdatedAt(utils::toIso8601Opt(optStr(row["updated_at"])));
    if (withCount) {
        avistamiento.setIdentificacionesCount(row["identificaciones_count"].as<int>());
        avistamiento.setEspecieSensible(row["especie_sensible"].as<bool>());
    }
    // Las consultas con RETURNING no traen `especie_sensible`: responden a
    // quien acaba de escribir la fila —su autor o la moderación—, y esos ven el
    // punto exacto de todas formas.
    return avistamiento;
}

Avistamiento PostgresAvistamientoRepository::create(const Avistamiento& avistamiento) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const std::string sql =
            std::string("INSERT INTO avistamientos (")
            + "especie_id, reino, nombre_sugerido, descripcion, foto_key, "
            + "geo_lat, geo_lng, precision_metros, precision_declarada, "
            + "observado_en, creado_por"
            + ") VALUES ("
            + quoteOptInt(txn, avistamiento.getEspecieId()) + ", "
            + txn.quote(reinoToString(avistamiento.getReino())) + "::reino_enum, "
            + quoteOptString(txn, avistamiento.getNombreSugerido()) + ", "
            + quoteOptString(txn, avistamiento.getDescripcion()) + ", "
            + txn.quote(avistamiento.getFotoKey()) + ", "
            + txn.quote(avistamiento.getGeoLat()) + ", "
            + txn.quote(avistamiento.getGeoLng()) + ", "
            + quoteOptDouble(txn, avistamiento.getPrecisionMetros()) + ", "
            + txn.quote(precisionDeclaradaToString(avistamiento.getPrecisionDeclarada()))
            + "::precision_declarada_enum, "
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
        if (filters.visibilidad) {
            where += " AND visibilidad = "
                + txn.quote(avistamientoVisibilidadToString(*filters.visibilidad))
                + "::avistamiento_visibilidad_enum";
        }
        if (filters.grado_identificacion) {
            where += " AND grado_identificacion = "
                + txn.quote(gradoIdentificacionToString(*filters.grado_identificacion))
                + "::grado_identificacion_enum";
        }

        const auto countResult = txn.exec("SELECT COUNT(*) FROM avistamientos a" + where);
        const int total = countResult[0][0].as<int>();

        // `id DESC` desempata en ambos casos: dos encuentros del mismo día
        // (o del mismo segundo, al importar) tienen que salir en un orden
        // estable o la paginación repite y saltea filas.
        const std::string ordenSql = filters.orden == OrdenAvistamiento::CreadoEn
                                         ? " ORDER BY created_at DESC, id DESC"
                                         : " ORDER BY observado_en DESC, id DESC";

        const std::string dataSql =
            std::string("SELECT ") + kSelectCols + kCountCol + sensibleCol(txn)
            + " FROM avistamientos a"
            + where
            + ordenSql
            + " LIMIT " + std::to_string(filters.limit)
            + " OFFSET " + std::to_string(filters.offset);

        const auto rows = txn.exec(dataSql);

        AvistamientoSearchResult result;
        result.total = total;
        result.data.reserve(rows.size());
        for (const auto& row : rows) {
            result.data.push_back(mapRowToAvistamiento(row, true));
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
            std::string("SELECT ") + kSelectCols + kCountCol + sensibleCol(txn)
                + " FROM avistamientos a WHERE a.id = $1",
            id);

        if (result.empty()) return std::nullopt;
        return mapRowToAvistamiento(result[0], true);
    } catch (const std::exception& error) {
        std::cerr << "Error al obtener avistamiento: " << error.what() << std::endl;
        throw;
    }
}

std::optional<Avistamiento> PostgresAvistamientoRepository::compartir(int id, int usuarioId) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Idempotente: volver a compartir algo ya público devuelve la fila igual.
        const auto result = txn.exec_params(
            std::string("UPDATE avistamientos SET visibilidad = 'publico'")
                + " WHERE id = $1 AND creado_por = $2"
                + " RETURNING " + kSelectCols,
            id, usuarioId);

        if (result.empty()) return std::nullopt;

        const auto count = txn.exec_params(
            "SELECT COUNT(*) FROM avistamiento_identificaciones"
            " WHERE avistamiento_id = $1 AND NOT retirada",
            id);

        txn.commit();
        auto avistamiento = mapRowToAvistamiento(result[0]);
        avistamiento.setIdentificacionesCount(count[0][0].as<int>());
        return avistamiento;
    } catch (const std::exception& error) {
        std::cerr << "Error al compartir avistamiento: " << error.what() << std::endl;
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

        // Un avistamiento moderado puede tener identificaciones previas, así
        // que el 0 por defecto del modelo mentiría. RETURNING no admite el
        // subselect correlacionado, de ahí la consulta aparte dentro de la
        // misma transacción.
        const auto count = txn.exec_params(
            "SELECT COUNT(*) FROM avistamiento_identificaciones"
            " WHERE avistamiento_id = $1 AND NOT retirada",
            id);

        txn.commit();
        auto avistamiento = mapRowToAvistamiento(result[0]);
        avistamiento.setIdentificacionesCount(count[0][0].as<int>());
        return avistamiento;
    } catch (const std::exception& error) {
        std::cerr << "Error al moderar avistamiento: " << error.what() << std::endl;
        throw;
    }
}

// Celdas agregadas para el mapa (Fase 9, PR 9).
//
// La agregación va entera en SQL: el punto del endpoint es no traer las filas.
// Dos niveles de GROUP BY —primero por (celda, especie), luego por celda— es lo
// que permite sacar la especie dominante sin un subselect correlacionado por
// celda.
//
// El tamaño de celda no es uniforme: una especie amenazada nunca se agrega más
// fino que kCeldaMinimaSensible aunque el zoom pida detalle. La clasificación
// usa el mismo patrón que utils::esEstadoConservacionSensible, generado desde
// la misma lista de tokens.
std::vector<CeldaMapa> PostgresAvistamientoRepository::mapa(const MapaFilters& filters) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const double celda = gradosPorCeldaSegunZoom(filters.zoom);
        const std::string celdaSql = txn.quote(celda);
        const std::string minimaSql = txn.quote(kCeldaMinimaSensible);

        std::string filtros;
        if (filters.reino) {
            filtros += " AND a.reino = " + txn.quote(reinoToString(*filters.reino))
                     + "::reino_enum";
        }
        if (filters.especie_id) {
            filtros += " AND a.especie_id = " + txn.quote(*filters.especie_id);
        }

        const std::string sql =
            std::string("WITH puntos AS ("
            " SELECT a.geo_lat, a.geo_lng, a.especie_id,"
            // COALESCE porque el LEFT JOIN deja `estado_conservacion` en NULL
            // para los encuentros sin especie asignada, y `~*` sobre NULL da
            // NULL, que luego sobrevive al bool_or y revienta al leerlo como
            // bool. Sin especie no hay nada que proteger: false.
            " COALESCE(e.estado_conservacion ~* ") + txn.quote(utils::patronSqlEstadoSensible())
            + ", false) AS sensible,"
            " CASE WHEN e.estado_conservacion ~* " + txn.quote(utils::patronSqlEstadoSensible())
            + " THEN GREATEST(" + celdaSql + "::numeric, " + minimaSql + "::numeric)"
            + " ELSE " + celdaSql + "::numeric END AS tam"
            + " FROM avistamientos a"
            + " LEFT JOIN especies e ON e.id = a.especie_id"
            + " WHERE a.estado = 'aprobado' AND a.visibilidad = 'publico'"
            + " AND a.geo_lat BETWEEN " + txn.quote(filters.min_lat)
            + " AND " + txn.quote(filters.max_lat)
            + " AND a.geo_lng BETWEEN " + txn.quote(filters.min_lng)
            + " AND " + txn.quote(filters.max_lng)
            + filtros
            + "), celdas AS ("
            + " SELECT floor(geo_lat / tam) * tam + tam / 2 AS lat,"
            + " floor(geo_lng / tam) * tam + tam / 2 AS lng,"
            + " tam, especie_id, bool_or(sensible) AS sensible, COUNT(*) AS n"
            + " FROM puntos GROUP BY 1, 2, 3, 4"
            + ") SELECT lat, lng, tam, SUM(n) AS total,"
            + " COUNT(*) FILTER (WHERE especie_id IS NOT NULL) AS especies_distintas,"
            + " (array_agg(especie_id ORDER BY n DESC, especie_id)"
            + "  FILTER (WHERE especie_id IS NOT NULL))[1] AS especie_dominante_id,"
            + " bool_or(sensible) AS sensible"
            + " FROM celdas GROUP BY lat, lng, tam"
            + " ORDER BY total DESC, lat, lng LIMIT 2000";

        pqxx::result result = txn.exec(sql);
        txn.commit();

        std::vector<CeldaMapa> celdas;
        celdas.reserve(result.size());
        for (const auto& row : result) {
            CeldaMapa item;
            item.lat = row["lat"].as<double>();
            item.lng = row["lng"].as<double>();
            item.grados = row["tam"].as<double>();
            item.total = row["total"].as<int>();
            item.especies_distintas = row["especies_distintas"].as<int>();
            item.especie_dominante_id = optInt(row["especie_dominante_id"]);
            item.sensible = row["sensible"].as<bool>();
            celdas.push_back(item);
        }
        return celdas;
    } catch (const std::exception& error) {
        std::cerr << "Error al agregar el mapa de avistamientos: " << error.what() << std::endl;
        throw;
    }
}
