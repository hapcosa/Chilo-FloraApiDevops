#include "../../include/repository/postgres_familia_repository.hpp"

#include <iostream>
#include <stdexcept>

PostgresFamiliaRepository::PostgresFamiliaRepository(
    std::shared_ptr<Database> database)
    : database(database) {}

Familia PostgresFamiliaRepository::mapRowToFamilia(const pqxx::row& row) {
  Familia familia(row["id"].as<int>(), row["nombre"].as<std::string>(),
                  row["descripcion"].as<std::string>());

  auto imagenes = getImagenes(familia.getId());
  for (const auto& imagen : imagenes) {
    if (imagen.getEsPrincipal()) familia.setImagenPrincipal(imagen.getUrl());
    familia.addImagenUrl(imagen.getUrl());
  }

  return familia;
}

std::vector<Familia> PostgresFamiliaRepository::getAll() {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    // Primero obtener todas las familias
    pqxx::result result = txn.exec("SELECT * FROM familias ORDER BY nombre");
    std::vector<Familia> familias;

    for (const auto& row : result) {
      familias.push_back(mapRowToFamilia(row));
    }

    return familias;
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener todas las familias: " << e.what()
              << std::endl;
    throw;
  }
}

std::optional<Familia> PostgresFamiliaRepository::findById(int id) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result result =
        txn.exec_params("SELECT * FROM familias WHERE id = $1", id);

    if (result.empty()) {
      return std::nullopt;
    }

    return mapRowToFamilia(result[0]);
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener familia por ID: " << e.what() << std::endl;
    throw;
  }
}

std::optional<Familia> PostgresFamiliaRepository::findByNombre(
    const std::string& nombre) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result result =
        txn.exec_params("SELECT * FROM familias WHERE nombre = $1", nombre);

    if (result.empty()) {
      return std::nullopt;
    }

    return mapRowToFamilia(result[0]);
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener familia por nombre: " << e.what()
              << std::endl;
    throw;
  }
}

Familia PostgresFamiliaRepository::create(const Familia& familia) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    // Validar nombre único
    pqxx::result check = txn.exec_params(
        "SELECT COUNT(*) FROM familias WHERE nombre = $1", familia.getNombre());

    if (check[0][0].as<int>() > 0) {
      throw std::invalid_argument("El nombre ya existe en la base de datos");
    }

    pqxx::result result = txn.exec_params(
        "INSERT INTO familias (nombre, descripcion) "
        "VALUES ($1, $2) RETURNING *",
        familia.getNombre(), familia.getDescripcion());

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

    // Verificar si existe la familia
    pqxx::result check = txn.exec_params(
        "SELECT COUNT(*) FROM familias WHERE id = $1", familia.getId());

    if (check[0][0].as<int>() == 0) {
      throw std::invalid_argument("Familia no encontrada");
    }

    // Validar nombre único (excepto para la misma familia)
    check = txn.exec_params(
        "SELECT COUNT(*) FROM familias WHERE nombre = $1 AND id != $2",
        familia.getNombre(), familia.getId());

    if (check[0][0].as<int>() > 0) {
      throw std::invalid_argument("El nombre ya existe para otra familia");
    }

    pqxx::result result = txn.exec_params(
        "UPDATE familias SET nombre = $1, descripcion = $2 "
        "WHERE id = $3 RETURNING *",
        familia.getNombre(), familia.getDescripcion(), familia.getId());

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

    pqxx::result result =
        txn.exec_params("DELETE FROM familias WHERE id = $1 RETURNING id", id);

    txn.commit();
    return !result.empty();
  } catch (const std::exception& e) {
    std::cerr << "Error al eliminar familia: " << e.what() << std::endl;
    throw;
  }
}

bool PostgresFamiliaRepository::agregarImagen(int familia_id,
                                              const std::string& imagen_url,
                                              bool es_principal) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    // Insertar la nueva imagen
    txn.exec_params(
        "INSERT INTO familia_imagenes (familia_id, url, es_principal) VALUES "
        "($1, $2, $3)",
        familia_id, imagen_url, es_principal);

    // Si es principal, desmarca las demás
    if (es_principal) {
      txn.exec_params(
          "UPDATE familia_imagenes SET es_principal = false WHERE familia_id = "
          "$1 AND url != $2",
          familia_id, imagen_url);
    }

    txn.commit();
    return true;
  } catch (const std::exception& e) {
    std::cout << "Error en base de datos: " << e.what() << std::endl;
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
    imagenes.emplace_back(row["id"].as<int>(), row["url"].as<std::string>(),
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
  } catch (const std::exception& e) {
    return false;
  }
}

bool PostgresFamiliaRepository::setImagenPrincipal(
    int familia_id, const std::string& imagen_url) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    // Desmarcar todas las imágenes como no principales
    txn.exec_params(
        "UPDATE familia_imagenes SET es_principal = false WHERE familia_id = "
        "$1",
        familia_id);

    // Marcar la imagen especificada como principal
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
