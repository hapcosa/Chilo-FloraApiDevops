#include "../../include/repository/postgres_familia_repository.hpp"

#include <iostream>
#include <stdexcept>

#include "../../include/utils/timestamps.hpp"

namespace {

constexpr const char* kSelectCols =
    "id, reino, nombre, descripcion, created_at";

std::optional<std::string> optStr(const pqxx::field& f) {
    if (f.is_null()) return std::nullopt;
    return std::string(f.c_str());
}

}  // namespace

PostgresFamiliaRepository::PostgresFamiliaRepository(
    std::shared_ptr<Database> database)
    : database(database) {}

Familia PostgresFamiliaRepository::mapRowToFamilia(const pqxx::row& row) {
    Familia f;
    f.setId(row["id"].as<int>());
    f.setReino(reinoFromString(row["reino"].c_str()));
    f.setNombre(row["nombre"].c_str());
    f.setDescripcion(row["descripcion"].is_null() ? "" : row["descripcion"].c_str());
    f.setCreatedAt(utils::toIso8601Opt(optStr(row["created_at"])));

    auto imagenes = getImagenes(f.getId());
    for (const auto& imagen : imagenes) {
        if (imagen.getEsPrincipal()) f.setImagenPrincipal(imagen.getUrl());
        f.addImagenUrl(imagen.getUrl());
    }
    return f;
}

std::vector<Familia> PostgresFamiliaRepository::getAll() {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec(
            std::string("SELECT ") + kSelectCols
            + " FROM familias ORDER BY reino, nombre");

        std::vector<Familia> familias;
        familias.reserve(result.size());
        for (const auto& row : result) {
            familias.push_back(mapRowToFamilia(row));
        }
        return familias;
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener todas las familias: " << e.what() << std::endl;
        throw;
    }
}

std::vector<Familia> PostgresFamiliaRepository::getByReino(Reino reino) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
            + " FROM familias WHERE reino = $1::reino_enum ORDER BY nombre",
            reinoToString(reino));

        std::vector<Familia> familias;
        familias.reserve(result.size());
        for (const auto& row : result) {
            familias.push_back(mapRowToFamilia(row));
        }
        return familias;
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener familias por reino: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Familia> PostgresFamiliaRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols + " FROM familias WHERE id = $1",
            id);
        if (result.empty()) return std::nullopt;
        return mapRowToFamilia(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener familia por ID: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Familia> PostgresFamiliaRepository::findByNombre(
    Reino reino, const std::string& nombre) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
            + " FROM familias WHERE reino = $1::reino_enum AND nombre = $2",
            reinoToString(reino), nombre);
        if (result.empty()) return std::nullopt;
        return mapRowToFamilia(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al obtener familia por nombre: " << e.what() << std::endl;
        throw;
    }
}

Familia PostgresFamiliaRepository::create(const Familia& familia) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result check = txn.exec_params(
            "SELECT COUNT(*) FROM familias WHERE reino = $1::reino_enum AND nombre = $2",
            reinoToString(familia.getReino()), familia.getNombre());
        if (check[0][0].as<int>() > 0) {
            throw std::invalid_argument(
                "Ya existe una familia con ese nombre en el reino indicado");
        }

        pqxx::result result = txn.exec_params(
            std::string("INSERT INTO familias (reino, nombre, descripcion) ")
            + "VALUES ($1::reino_enum, $2, $3) RETURNING " + kSelectCols,
            reinoToString(familia.getReino()),
            familia.getNombre(),
            familia.getDescripcion());

        txn.commit();
        return mapRowToFamilia(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al crear familia: " << e.what() << std::endl;
        throw;
    }
}

Familia PostgresFamiliaRepository::update(const Familia& familia) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result check = txn.exec_params(
            "SELECT COUNT(*) FROM familias WHERE id = $1", familia.getId());
        if (check[0][0].as<int>() == 0) {
            throw std::invalid_argument("Familia no encontrada");
        }

        check = txn.exec_params(
            "SELECT COUNT(*) FROM familias "
            "WHERE reino = $1::reino_enum AND nombre = $2 AND id != $3",
            reinoToString(familia.getReino()), familia.getNombre(), familia.getId());
        if (check[0][0].as<int>() > 0) {
            throw std::invalid_argument(
                "Ya existe otra familia con ese (reino, nombre)");
        }

        pqxx::result result = txn.exec_params(
            std::string("UPDATE familias ")
            + "SET reino = $1::reino_enum, nombre = $2, descripcion = $3 "
            + "WHERE id = $4 RETURNING " + kSelectCols,
            reinoToString(familia.getReino()),
            familia.getNombre(),
            familia.getDescripcion(),
            familia.getId());

        txn.commit();
        return mapRowToFamilia(result[0]);
    } catch (const std::exception& e) {
        std::cerr << "Error al actualizar familia: " << e.what() << std::endl;
        throw;
    }
}

bool PostgresFamiliaRepository::remove(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "DELETE FROM familias WHERE id = $1 RETURNING id", id);
        txn.commit();
        return !result.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error al eliminar familia: " << e.what() << std::endl;
        throw;
    }
}

// =============================================================================
// Imágenes legacy: tabla familia_imagenes. Intacto. Migra en Fase 2.
// =============================================================================

bool PostgresFamiliaRepository::agregarImagen(int familia_id,
                                               const std::string& imagen_url,
                                               bool es_principal) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        txn.exec_params(
            "INSERT INTO familia_imagenes (familia_id, url, es_principal) "
            "VALUES ($1, $2, $3)",
            familia_id, imagen_url, es_principal);

        if (es_principal) {
            txn.exec_params(
                "UPDATE familia_imagenes SET es_principal = false "
                "WHERE familia_id = $1 AND url != $2",
                familia_id, imagen_url);
        }

        txn.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error en base de datos: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Imagen> PostgresFamiliaRepository::getImagenes(int familia_id) {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    std::vector<Imagen> imagenes;
    auto result = txn.exec_params(
        "SELECT id, url, es_principal FROM familia_imagenes "
        "WHERE familia_id = $1 ORDER BY es_principal DESC",
        familia_id);

    for (const auto& row : result) {
        imagenes.emplace_back(row["id"].as<int>(),
                              row["url"].as<std::string>(),
                              row["es_principal"].as<bool>());
    }
    return imagenes;
}

bool PostgresFamiliaRepository::eliminarImagen(int familia_id,
                                                const std::string& imagen_url) {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    try {
        auto result = txn.exec_params(
            "DELETE FROM familia_imagenes WHERE familia_id = $1 AND url = $2",
            familia_id, imagen_url);
        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception&) {
        return false;
    }
}

bool PostgresFamiliaRepository::setImagenPrincipal(int familia_id,
                                                    const std::string& imagen_url) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        txn.exec_params(
            "UPDATE familia_imagenes SET es_principal = false WHERE familia_id = $1",
            familia_id);

        auto result = txn.exec_params(
            "UPDATE familia_imagenes SET es_principal = true "
            "WHERE familia_id = $1 AND url = $2",
            familia_id, imagen_url);

        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error al establecer imagen principal: " << e.what()
                  << std::endl;
        return false;
    }
}
