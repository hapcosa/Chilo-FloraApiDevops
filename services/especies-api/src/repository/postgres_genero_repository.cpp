#include "../../include/repository/postgres_genero_repository.hpp"

#include <iostream>
#include <stdexcept>

namespace {

constexpr const char* kSelectCols =
    "id, familia_id, nombre, descripcion, created_at";

std::optional<std::string> optStr(const pqxx::field& f) {
    if (f.is_null()) return std::nullopt;
    return std::string(f.c_str());
}

}  // namespace

PostgresGeneroRepository::PostgresGeneroRepository(
    std::shared_ptr<Database> database)
    : database(database) {}

Genero PostgresGeneroRepository::mapRowToGenero(const pqxx::row& row) {
    Genero g;
    g.setId(row["id"].as<int>());
    g.setFamiliaId(row["familia_id"].as<int>());
    g.setNombre(row["nombre"].c_str());
    g.setDescripcion(row["descripcion"].is_null() ? "" : row["descripcion"].c_str());
    g.setCreatedAt(optStr(row["created_at"]));

    auto imagenes = getImagenes(g.getId());
    for (const auto& imagen : imagenes) {
        if (imagen.getEsPrincipal()) g.setImagenPrincipal(imagen.getUrl());
        g.addImagenUrl(imagen.getUrl());
    }
    return g;
}

std::vector<Genero> PostgresGeneroRepository::getAll() {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec(
            std::string("SELECT ") + kSelectCols + " FROM generos ORDER BY nombre");
        std::vector<Genero> generos;
        generos.reserve(result.size());
        for (const auto& row : result) {
            generos.push_back(mapRowToGenero(row));
        }
        return generos;
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener todos los géneros: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Genero> PostgresGeneroRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols + " FROM generos WHERE id = $1", id);
        if (result.empty()) return std::nullopt;
        return mapRowToGenero(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener género por ID: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Genero> PostgresGeneroRepository::findByNombre(
    int familia_id, const std::string& nombre) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
            + " FROM generos WHERE familia_id = $1 AND nombre = $2",
            familia_id, nombre);
        if (result.empty()) return std::nullopt;
        return mapRowToGenero(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener género por nombre: " << e.what() << std::endl;
        throw;
    }
}

std::vector<Genero> PostgresGeneroRepository::getByFamilia(
    const std::string& familia) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "SELECT g.id, g.familia_id, g.nombre, g.descripcion, g.created_at "
            "FROM generos g JOIN familias f ON g.familia_id = f.id "
            "WHERE f.nombre = $1 ORDER BY g.nombre",
            familia);

        std::vector<Genero> generos;
        generos.reserve(result.size());
        for (const auto& row : result) {
            generos.push_back(mapRowToGenero(row));
        }
        return generos;
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener géneros por familia: " << e.what() << std::endl;
        throw;
    }
}

Genero PostgresGeneroRepository::create(const Genero& genero) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Unicidad (familia_id, nombre) — clave nueva tras 0002_multi_reino.
        pqxx::result checkNombre = txn.exec_params(
            "SELECT COUNT(*) FROM generos WHERE familia_id = $1 AND LOWER(nombre) = LOWER($2)",
            genero.getFamiliaId(), genero.getNombre());
        if (checkNombre[0][0].as<int>() > 0) {
            throw std::invalid_argument(
                "Ya existe un género con nombre '" + genero.getNombre()
                + "' en esa familia");
        }

        pqxx::result checkFamilia = txn.exec_params(
            "SELECT id FROM familias WHERE id = $1", genero.getFamiliaId());
        if (checkFamilia.empty()) {
            throw std::runtime_error(
                "No existe familia con ID " + std::to_string(genero.getFamiliaId())
                + ". Cree primero la familia.");
        }

        pqxx::result result = txn.exec_params(
            std::string("INSERT INTO generos (nombre, descripcion, familia_id) ")
            + "VALUES ($1, $2, $3) RETURNING " + kSelectCols,
            genero.getNombre(), genero.getDescripcion(), genero.getFamiliaId());

        txn.commit();
        return mapRowToGenero(result[0]);

    } catch (const pqxx::foreign_key_violation& e) {
        std::cerr << "Error de clave foránea: " << e.what() << std::endl;
        throw std::runtime_error("La familia especificada no existe.");
    } catch (const std::exception& e) {
        std::cerr << "Error al crear género: " << e.what() << std::endl;
        throw;
    }
}

Genero PostgresGeneroRepository::update(const Genero& genero) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result check = txn.exec_params(
            "SELECT COUNT(*) FROM generos WHERE id = $1", genero.getId());
        if (check[0][0].as<int>() == 0) {
            throw std::invalid_argument("Género no encontrado");
        }

        check = txn.exec_params(
            "SELECT COUNT(*) FROM generos "
            "WHERE familia_id = $1 AND nombre = $2 AND id != $3",
            genero.getFamiliaId(), genero.getNombre(), genero.getId());
        if (check[0][0].as<int>() > 0) {
            throw std::invalid_argument(
                "Ya existe otro género con ese nombre en la misma familia");
        }

        pqxx::result result = txn.exec_params(
            std::string("UPDATE generos SET nombre = $1, descripcion = $2, familia_id = $3 ")
            + "WHERE id = $4 RETURNING " + kSelectCols,
            genero.getNombre(), genero.getDescripcion(),
            genero.getFamiliaId(), genero.getId());

        txn.commit();
        return mapRowToGenero(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al actualizar género: " << e.what() << std::endl;
        throw;
    }
}

bool PostgresGeneroRepository::remove(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec_params(
            "DELETE FROM generos WHERE id = $1 RETURNING id", id);
        txn.commit();
        return !result.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error al eliminar género: " << e.what() << std::endl;
        throw;
    }
}

// =============================================================================
// Imágenes legacy: tabla genero_imagenes. Intacto. Migra en Fase 2.
// =============================================================================

bool PostgresGeneroRepository::agregarImagen(int genero_id,
                                              const std::string& imagen_url,
                                              bool es_principal) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        txn.exec_params(
            "INSERT INTO genero_imagenes (genero_id, url, es_principal) "
            "VALUES ($1, $2, $3)",
            genero_id, imagen_url, es_principal);
        if (es_principal) {
            txn.exec_params(
                "UPDATE genero_imagenes SET es_principal = false "
                "WHERE genero_id = $1 AND url != $2",
                genero_id, imagen_url);
        }
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error en base de datos: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Imagen> PostgresGeneroRepository::getImagenes(int genero_id) {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    std::vector<Imagen> imagenes;
    auto result = txn.exec_params(
        "SELECT id, url, es_principal FROM genero_imagenes "
        "WHERE genero_id = $1 ORDER BY es_principal DESC",
        genero_id);
    for (const auto& row : result) {
        imagenes.emplace_back(row["id"].as<int>(),
                              row["url"].as<std::string>(),
                              row["es_principal"].as<bool>());
    }
    return imagenes;
}

bool PostgresGeneroRepository::eliminarImagen(int genero_id,
                                               const std::string& imagen_url) {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    try {
        auto result = txn.exec_params(
            "DELETE FROM genero_imagenes WHERE genero_id = $1 AND url = $2",
            genero_id, imagen_url);
        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception&) {
        return false;
    }
}

bool PostgresGeneroRepository::setImagenPrincipal(int genero_id,
                                                   const std::string& imagen_url) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        txn.exec_params(
            "UPDATE genero_imagenes SET es_principal = false WHERE genero_id = $1",
            genero_id);
        auto result = txn.exec_params(
            "UPDATE genero_imagenes SET es_principal = true "
            "WHERE genero_id = $1 AND url = $2",
            genero_id, imagen_url);
        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error al establecer imagen principal: " << e.what()
                  << std::endl;
        return false;
    }
}
