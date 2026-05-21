#include "../../include/repository/postgres_genero_repository.hpp"

#include <iostream>
#include <stdexcept>

PostgresGeneroRepository::PostgresGeneroRepository(
    std::shared_ptr<Database> database)
    : database(database) {}

Genero PostgresGeneroRepository::mapRowToGenero(const pqxx::row& row) {
  // CORREGIDO: familia_id en lugar de Familia_id
  Genero genero(row["id"].as<int>(), row["nombre"].as<std::string>(),
                row["descripcion"].as<std::string>(),
                row["familia_id"].as<int>());
  auto imagenes = getImagenes(genero.getId());
  for (const auto& imagen : imagenes) {
    if (imagen.getEsPrincipal()) genero.setImagenPrincipal(imagen.getUrl());
    genero.addImagenUrl(imagen.getUrl());
  }
  return genero;
}

std::vector<Genero> PostgresGeneroRepository::getAll() {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec("SELECT * FROM generos ORDER BY nombre");
    std::vector<Genero> generos;

    for (const auto& row : result) {
      generos.push_back(mapRowToGenero(row));
    }

    return generos;
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener todos los géneros: " << e.what()
              << std::endl;
    throw;
  }
}

std::optional<Genero> PostgresGeneroRepository::findById(int id) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result result =
        txn.exec_params("SELECT * FROM generos WHERE id = $1", id);

    if (result.empty()) {
      return std::nullopt;
    }

    return mapRowToGenero(result[0]);
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener género por ID: " << e.what() << std::endl;
    throw;
  }
}

std::optional<Genero> PostgresGeneroRepository::findByNombre(
    const std::string& nombre) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result result =
        txn.exec_params("SELECT * FROM generos WHERE nombre = $1", nombre);

    if (result.empty()) {
      return std::nullopt;
    }

    return mapRowToGenero(result[0]);
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener género por nombre: " << e.what()
              << std::endl;
    throw;
  }
}

std::vector<Genero> PostgresGeneroRepository::getByFamilia(
    const std::string& familia) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result result = txn.exec_params(
        "SELECT generos.* FROM generos JOIN familias ON generos.familia_id = "
        "familias.id "
        "WHERE familias.nombre = $1",
        familia);

    std::vector<Genero> generos;
    for (const auto& row : result) {
      generos.push_back(mapRowToGenero(row));
    }

    return generos;
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener géneros por familia: " << e.what()
              << std::endl;
    throw;
  }
}

Genero PostgresGeneroRepository::create(const Genero& genero) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    pqxx::result checkNombre = txn.exec_params(
        "SELECT COUNT(*) FROM generos WHERE LOWER(nombre) = LOWER($1)",
        genero.getNombre());

    if (checkNombre[0][0].as<int>() > 0) {
      throw std::invalid_argument("Ya existe un género con el nombre '" +
                                  genero.getNombre() + "'");
    }

    // 2. Validar que la familia exista (con mensaje más descriptivo)
    pqxx::result checkFamilia = txn.exec_params(
        "SELECT id, nombre FROM familias WHERE id = $1", genero.getFamiliaId());

    if (checkFamilia.empty()) {
      throw std::runtime_error("No se encontró familia con ID: " +
                               std::to_string(genero.getFamiliaId()) +
                               ". Primero debe crear la familia.");
    }

    // 3. Insertar el nuevo género
    pqxx::result result = txn.exec_params(
        "INSERT INTO generos (nombre, descripcion, familia_id) "
        "VALUES ($1, $2, $3) RETURNING id, nombre, descripcion, familia_id",
        genero.getNombre(), genero.getDescripcion(), genero.getFamiliaId());

    txn.commit();

    // 4. Mapear y retornar el resultado
    return mapRowToGenero(result[0]);

  } catch (const pqxx::foreign_key_violation& e) {
    // Captura específica para errores de FK
    std::cerr << "Error de clave foránea: " << e.what() << std::endl;
    throw std::runtime_error(
        "Error: La familia especificada no existe. Verifique el ID.");

  } catch (const std::exception& e) {
    std::cerr << "Error al crear género: " << e.what() << std::endl;
    throw;  // Relanzar para manejo superior
  }
}

Genero PostgresGeneroRepository::update(const Genero& genero) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    // Verificar si existe el género
    pqxx::result check = txn.exec_params(
        "SELECT COUNT(*) FROM generos WHERE id = $1", genero.getId());

    if (check[0][0].as<int>() == 0) {
      throw std::invalid_argument("Género no encontrado");
    }

    // Validar nombre único (excepto para el mismo género)
    check = txn.exec_params(
        "SELECT COUNT(*) FROM generos WHERE nombre = $1 AND id != $2",
        genero.getNombre(), genero.getId());

    if (check[0][0].as<int>() > 0) {
      throw std::invalid_argument("El nombre ya existe para otro género");
    }

    pqxx::result result = txn.exec_params(
        "UPDATE generos SET nombre = $1, descripcion = $2, familia_id = $3 "
        "WHERE id = $4 RETURNING *",
        genero.getNombre(), genero.getDescripcion(), genero.getFamiliaId(),
        genero.getId());

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

    pqxx::result result =
        txn.exec_params("DELETE FROM generos WHERE id = $1 RETURNING id", id);

    txn.commit();
    return !result.empty();
  } catch (const std::exception& e) {
    std::cerr << "Error al eliminar género: " << e.what() << std::endl;
    throw;
  }
}

bool PostgresGeneroRepository::agregarImagen(int genero_id,
                                             const std::string& imagen_url,
                                             bool es_principal) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "INSERT INTO genero_imagenes (genero_id, url, es_principal) VALUES "
        "($1, $2, $3)",
        genero_id, imagen_url, es_principal);
    if (es_principal) {
      txn.exec_params(
          "UPDATE genero_imagenes SET es_principal = false WHERE genero_id = "
          "$1 AND url != $2",
          genero_id, imagen_url);
    }
    txn.commit();
    return true;
  } catch (const std::exception& e) {
    std::cout << "Error en base de datos: " << e.what() << std::endl;
    throw;
  }
}

std::vector<Imagen> PostgresGeneroRepository::getImagenes(int genero_id) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    std::vector<Imagen> imagenes;
    auto result = txn.exec_params(
        "SELECT id, url, es_principal FROM genero_imagenes "
        "WHERE genero_id = $1 ORDER BY es_principal DESC",
        genero_id);
    for (const auto& row : result) {
      imagenes.emplace_back(row["id"].as<int>(), row["url"].as<std::string>(),
                            row["es_principal"].as<bool>());
    }
    return imagenes;

  } catch (const std::exception& e) {
    std::cout << "Error en base de datos: " << e.what() << std::endl;
    throw;
  }
}

bool PostgresGeneroRepository::eliminarImagen(int genero_id,
                                              const std::string& imagen_url) {
  auto conn = database->createConnection();
  pqxx::work txn(*conn);
  try {
    // CORREGIDO: url en lugar de imagen_url
    auto result = txn.exec_params(
        "DELETE FROM genero_imagenes WHERE genero_id = $1 AND url = $2",
        genero_id, imagen_url);
    txn.commit();
    return result.affected_rows() > 0;
  } catch (const std::exception& e) {
    return false;
  }
}

bool PostgresGeneroRepository::setImagenPrincipal(
    int genero_id, const std::string& imagen_url) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    // Desmarcar todas las imágenes como no principales
    txn.exec_params(
        "UPDATE genero_imagenes SET es_principal = false WHERE genero_id = $1",
        genero_id);

    // Marcar la imagen especificada como principal
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
