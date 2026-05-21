#include "../../include/repository/postgresql_especie_repository.hpp"

#include <iostream>
#include <stdexcept>

namespace {

// Columnas seleccionadas en orden estable para reutilizar en SELECT,
// SELECT * y mapRowToEspecie. Excluye la lectura de imágenes legacy
// (especies_imagenes) que se hace aparte.
constexpr const char* kSelectCols =
    "id, reino, genero_id, nombre_cientifico, nombre_comun, autor_cientifico, "
    "descripcion, habitat, distribucion_chiloe, endemica, estado_conservacion, "
    "fuentes::text AS fuentes_text, geo_lat, geo_lng, "
    "atributos_especificos::text AS atributos_text, "
    "foto_portada_key, fotos_keys::text AS fotos_keys_text, "
    "creado_por, revisado_por, "
    "fecha_revision, created_at, updated_at";

nlohmann::json parseJsonbText(const pqxx::field& f,
                              const nlohmann::json& fallback) {
    if (f.is_null()) return fallback;
    return nlohmann::json::parse(f.c_str());
}

std::optional<std::string> optStr(const pqxx::field& f) {
    if (f.is_null()) return std::nullopt;
    return std::string(f.c_str());
}

std::optional<int> optInt(const pqxx::field& f) {
    if (f.is_null()) return std::nullopt;
    return f.as<int>();
}

std::optional<double> optDouble(const pqxx::field& f) {
    if (f.is_null()) return std::nullopt;
    return f.as<double>();
}

}  // namespace

PostgreSQLEspecieRepository::PostgreSQLEspecieRepository(
    std::shared_ptr<Database> database)
    : database(database) {}

Especie PostgreSQLEspecieRepository::mapRowToEspecie(const pqxx::row& row) {
    Especie e;
    e.setId(row["id"].as<int>());
    e.setReino(reinoFromString(row["reino"].c_str()));
    e.setGeneroId(row["genero_id"].as<int>());
    e.setNombreCientifico(row["nombre_cientifico"].c_str());
    e.setNombreComun(row["nombre_comun"].is_null() ? "" : row["nombre_comun"].c_str());
    e.setAutorCientifico(
        row["autor_cientifico"].is_null() ? "" : row["autor_cientifico"].c_str());
    e.setDescripcion(row["descripcion"].is_null() ? "" : row["descripcion"].c_str());
    e.setHabitat(row["habitat"].is_null() ? "" : row["habitat"].c_str());
    e.setDistribucionChiloe(
        row["distribucion_chiloe"].is_null() ? "" : row["distribucion_chiloe"].c_str());
    e.setEndemica(row["endemica"].as<bool>());
    e.setEstadoConservacion(
        row["estado_conservacion"].is_null() ? "" : row["estado_conservacion"].c_str());

    e.setFuentes(parseJsonbText(row["fuentes_text"], nlohmann::json::array()));
    e.setGeoLat(optDouble(row["geo_lat"]));
    e.setGeoLng(optDouble(row["geo_lng"]));
    e.setAtributosEspecificos(
        parseJsonbText(row["atributos_text"], nlohmann::json::object()));
    e.setFotoPortadaKey(optStr(row["foto_portada_key"]));
    e.setFotosKeys(parseJsonbText(row["fotos_keys_text"], nlohmann::json::array()));
    e.setCreadoPor(optInt(row["creado_por"]));
    e.setRevisadoPor(optInt(row["revisado_por"]));
    e.setFechaRevision(optStr(row["fecha_revision"]));
    e.setCreatedAt(optStr(row["created_at"]));
    e.setUpdatedAt(optStr(row["updated_at"]));

    // Imágenes legacy: tabla especies_imagenes. Se mantiene este PR.
    auto imagenes = getImagenes(e.getId());
    for (const auto& url : imagenes) {
        e.addImagenUrl(url);
    }

    return e;
}

std::vector<Especie> PostgreSQLEspecieRepository::getAll() {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec(
            std::string("SELECT ") + kSelectCols
            + " FROM especies ORDER BY nombre_cientifico");

        std::vector<Especie> especies;
        especies.reserve(result.size());
        for (const auto& row : result) {
            especies.push_back(mapRowToEspecie(row));
        }
        return especies;
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener todas las especies: " << e.what() << std::endl;
        throw;
    }
}

std::vector<Especie> PostgreSQLEspecieRepository::getByReino(Reino reino) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
            + " FROM especies WHERE reino = $1::reino_enum"
            + " ORDER BY nombre_cientifico",
            reinoToString(reino));

        std::vector<Especie> especies;
        especies.reserve(result.size());
        for (const auto& row : result) {
            especies.push_back(mapRowToEspecie(row));
        }
        return especies;
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener especies por reino: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Especie> PostgreSQLEspecieRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols + " FROM especies WHERE id = $1",
            id);

        if (result.empty()) return std::nullopt;
        return mapRowToEspecie(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener especie por ID: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Especie> PostgreSQLEspecieRepository::getByNombreCientifico(
    const std::string& nombre) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
            + " FROM especies WHERE nombre_cientifico ILIKE $1",
            "%" + nombre + "%");

        if (result.empty()) return std::nullopt;
        return mapRowToEspecie(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al buscar especies por nombre científico: " << e.what()
                  << std::endl;
        throw;
    }
}

std::vector<Especie> PostgreSQLEspecieRepository::getByGenero(
    const std::string& nombre) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
            + " FROM especies e JOIN generos g ON e.genero_id = g.id"
            + " WHERE g.nombre = $1 ORDER BY nombre_cientifico",
            nombre);

        std::vector<Especie> especies;
        especies.reserve(result.size());
        for (const auto& row : result) {
            especies.push_back(mapRowToEspecie(row));
        }
        return especies;
    } catch (const std::exception& e) {
        std::cerr << "Error al buscar especies por género: " << e.what() << std::endl;
        throw;
    }
}

Especie PostgreSQLEspecieRepository::create(const Especie& especie) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Unicidad de nombre_cientifico
        pqxx::result check = txn.exec_params(
            "SELECT COUNT(*) FROM especies WHERE nombre_cientifico = $1",
            especie.getNombreCientifico());
        if (check[0][0].as<int>() > 0) {
            throw std::invalid_argument(
                "El nombre científico ya existe en la base de datos");
        }

        pqxx::result result = txn.exec_params(
            std::string("INSERT INTO especies (")
                + "reino, genero_id, nombre_cientifico, nombre_comun, "
                + "autor_cientifico, descripcion, habitat, distribucion_chiloe, "
                + "endemica, estado_conservacion, fuentes, geo_lat, geo_lng, "
                + "atributos_especificos, foto_portada_key, fotos_keys, "
                + "creado_por, revisado_por, fecha_revision"
                + ") VALUES ("
                + "$1::reino_enum, $2, $3, $4, $5, $6, $7, $8, $9, $10, "
                + "$11::jsonb, $12, $13, $14::jsonb, $15, $16::jsonb, $17, $18, $19"
                + ") RETURNING " + kSelectCols,
            reinoToString(especie.getReino()),
            especie.getGeneroId(),
            especie.getNombreCientifico(),
            especie.getNombreComun(),
            especie.getAutorCientifico(),
            especie.getDescripcion(),
            especie.getHabitat(),
            especie.getDistribucionChiloe(),
            especie.isEndemica(),
            especie.getEstadoConservacion(),
            especie.getFuentes().dump(),
            especie.getGeoLat(),
            especie.getGeoLng(),
            especie.getAtributosEspecificos().dump(),
            especie.getFotoPortadaKey(),
            especie.getFotosKeys().dump(),
            especie.getCreadoPor(),
            especie.getRevisadoPor(),
            especie.getFechaRevision());

        txn.commit();
        return mapRowToEspecie(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al crear especie: " << e.what() << std::endl;
        throw;
    }
}

Especie PostgreSQLEspecieRepository::update(const Especie& especie) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result check = txn.exec_params(
            "SELECT COUNT(*) FROM especies WHERE id = $1", especie.getId());
        if (check[0][0].as<int>() == 0) {
            throw std::invalid_argument(
                "Especie no encontrada con ID: " + std::to_string(especie.getId()));
        }

        check = txn.exec_params(
            "SELECT COUNT(*) FROM especies WHERE nombre_cientifico = $1 AND id != $2",
            especie.getNombreCientifico(), especie.getId());
        if (check[0][0].as<int>() > 0) {
            throw std::invalid_argument(
                "El nombre científico ya existe para otra especie");
        }

        pqxx::result result = txn.exec_params(
            std::string("UPDATE especies SET ")
                + "reino = $1::reino_enum, genero_id = $2, nombre_cientifico = $3, "
                + "nombre_comun = $4, autor_cientifico = $5, descripcion = $6, "
                + "habitat = $7, distribucion_chiloe = $8, endemica = $9, "
                + "estado_conservacion = $10, fuentes = $11::jsonb, geo_lat = $12, "
                + "geo_lng = $13, atributos_especificos = $14::jsonb, "
                + "foto_portada_key = $15, fotos_keys = $16::jsonb, "
                + "creado_por = $17, revisado_por = $18, fecha_revision = $19 "
                + "WHERE id = $20 RETURNING " + kSelectCols,
            reinoToString(especie.getReino()),
            especie.getGeneroId(),
            especie.getNombreCientifico(),
            especie.getNombreComun(),
            especie.getAutorCientifico(),
            especie.getDescripcion(),
            especie.getHabitat(),
            especie.getDistribucionChiloe(),
            especie.isEndemica(),
            especie.getEstadoConservacion(),
            especie.getFuentes().dump(),
            especie.getGeoLat(),
            especie.getGeoLng(),
            especie.getAtributosEspecificos().dump(),
            especie.getFotoPortadaKey(),
            especie.getFotosKeys().dump(),
            especie.getCreadoPor(),
            especie.getRevisadoPor(),
            especie.getFechaRevision(),
            especie.getId());

        txn.commit();
        return mapRowToEspecie(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al actualizar especie: " << e.what() << std::endl;
        throw;
    }
}

bool PostgreSQLEspecieRepository::remove(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "DELETE FROM especies WHERE id = $1 RETURNING id", id);

        txn.commit();
        return !result.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error al eliminar especie: " << e.what() << std::endl;
        throw;
    }
}

// =============================================================================
// Imágenes legacy: tabla especies_imagenes. Intacto en este PR. Será
// migrado a foto_portada_key + fotos_keys en la Fase 2 (object storage).
// =============================================================================

bool PostgreSQLEspecieRepository::agregarImagen(int especie_id,
                                                 const std::string& imagen_url,
                                                 bool es_principal) {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    try {
        if (es_principal) {
            txn.exec_params(
                "UPDATE especies_imagenes SET es_principal = false "
                "WHERE especie_id = $1",
                especie_id);
        }

        txn.exec_params(
            "INSERT INTO especies_imagenes (especie_id, url, es_principal) "
            "VALUES ($1, $2, $3)",
            especie_id, imagen_url, es_principal);

        txn.commit();
        return true;
    } catch (const std::exception&) {
        txn.abort();
        return false;
    }
}

std::vector<std::string> PostgreSQLEspecieRepository::getImagenes(int especie_id) {
    auto conn = database->createConnection();
    pqxx::read_transaction txn(*conn);

    std::vector<std::string> imagenes;
    auto result = txn.exec_params(
        "SELECT url FROM especies_imagenes "
        "WHERE especie_id = $1 ORDER BY es_principal DESC",
        especie_id);

    for (const auto& row : result) {
        imagenes.push_back(row["url"].as<std::string>());
    }
    return imagenes;
}

bool PostgreSQLEspecieRepository::eliminarImagen(int especie_id,
                                                  const std::string& imagen_url) {
    auto conn = database->createConnection();
    try {
        pqxx::work txn(*conn);
        // NOTE: la tabla legacy se llama `especies_imagenes` y la columna `url`
        // (no `especie_imagenes` ni `imagen_url`). Bug del código previo.
        auto result = txn.exec_params(
            "DELETE FROM especies_imagenes WHERE especie_id = $1 AND url = $2",
            especie_id, imagen_url);
        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception&) {
        return false;
    }
}

bool PostgreSQLEspecieRepository::setImagenPrincipal(int especie_id,
                                                      const std::string& imagen_url) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        txn.exec_params(
            "UPDATE especies_imagenes SET es_principal = false "
            "WHERE especie_id = $1",
            especie_id);

        auto result = txn.exec_params(
            "UPDATE especies_imagenes SET es_principal = true "
            "WHERE especie_id = $1 AND url = $2",
            especie_id, imagen_url);

        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error al establecer imagen principal: " << e.what() << std::endl;
        return false;
    }
}
