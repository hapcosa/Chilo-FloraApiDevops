#include "../../include/repository/postgres_categoria_moderacion_repository.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

constexpr const char* kSelectCols = "id, reino, nombre, descripcion, created_at";

std::optional<std::string> optStr(const pqxx::field& field) {
  if (field.is_null()) return std::nullopt;
  return std::string(field.c_str());
}

}  // namespace

PostgresCategoriaModeracionRepository::PostgresCategoriaModeracionRepository(
    std::shared_ptr<Database> database)
    : database(std::move(database)) {}

CategoriaModeracion PostgresCategoriaModeracionRepository::mapRowToCategoria(
    const pqxx::row& row) {
  CategoriaModeracion categoria;
  categoria.setId(row["id"].as<int>());
  categoria.setReino(reinoFromString(row["reino"].c_str()));
  categoria.setNombre(row["nombre"].c_str());
  categoria.setDescripcion(optStr(row["descripcion"]));
  categoria.setCreatedAt(optStr(row["created_at"]));
  return categoria;
}

CategoriaModeracion PostgresCategoriaModeracionRepository::create(
    const CategoriaModeracion& categoria) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result check = txn.exec_params(
        "SELECT COUNT(*) FROM categorias_moderacion WHERE reino = $1::reino_enum AND nombre = $2",
        reinoToString(categoria.getReino()), categoria.getNombre());
    if (check[0][0].as<int>() > 0) {
      throw std::invalid_argument(
          "Ya existe una categoría de moderación con ese nombre en el reino indicado");
    }

    pqxx::result result = txn.exec_params(
        std::string("INSERT INTO categorias_moderacion (reino, nombre, descripcion) ") +
            "VALUES ($1::reino_enum, $2, $3) RETURNING " + kSelectCols,
        reinoToString(categoria.getReino()), categoria.getNombre(), categoria.getDescripcion());

    txn.commit();
    return mapRowToCategoria(result[0]);
  } catch (const std::exception& e) {
    std::cerr << "Error al crear categoría de moderación: " << e.what() << std::endl;
    throw;
  }
}

std::vector<CategoriaModeracion> PostgresCategoriaModeracionRepository::findAll() {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    const auto rows = txn.exec(
        std::string("SELECT ") + kSelectCols + " FROM categorias_moderacion ORDER BY reino, nombre");

    std::vector<CategoriaModeracion> result;
    result.reserve(rows.size());
    for (const auto& row : rows) {
      result.push_back(mapRowToCategoria(row));
    }
    return result;
  } catch (const std::exception& e) {
    std::cerr << "Error al listar categorías de moderación: " << e.what() << std::endl;
    throw;
  }
}

std::optional<CategoriaModeracion> PostgresCategoriaModeracionRepository::findById(int id) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    const auto result = txn.exec_params(
        std::string("SELECT ") + kSelectCols + " FROM categorias_moderacion WHERE id = $1", id);

    if (result.empty()) return std::nullopt;
    return mapRowToCategoria(result[0]);
  } catch (const std::exception& e) {
    std::cerr << "Error al obtener categoría de moderación: " << e.what() << std::endl;
    throw;
  }
}

CategoriaModeracion PostgresCategoriaModeracionRepository::update(
    const CategoriaModeracion& categoria) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result check =
        txn.exec_params("SELECT COUNT(*) FROM categorias_moderacion WHERE id = $1", categoria.getId());
    if (check[0][0].as<int>() == 0) {
      throw std::invalid_argument("Categoría de moderación no encontrada");
    }

    check = txn.exec_params(
        "SELECT COUNT(*) FROM categorias_moderacion "
        "WHERE reino = $1::reino_enum AND nombre = $2 AND id != $3",
        reinoToString(categoria.getReino()), categoria.getNombre(), categoria.getId());
    if (check[0][0].as<int>() > 0) {
      throw std::invalid_argument(
          "Ya existe otra categoría de moderación con ese nombre en el reino indicado");
    }

    pqxx::result result = txn.exec_params(
        std::string("UPDATE categorias_moderacion SET ") +
            "reino = $1::reino_enum, nombre = $2, descripcion = $3 " +
            "WHERE id = $4 RETURNING " + kSelectCols,
        reinoToString(categoria.getReino()), categoria.getNombre(), categoria.getDescripcion(),
        categoria.getId());

    txn.commit();
    return mapRowToCategoria(result[0]);
  } catch (const std::exception& e) {
    std::cerr << "Error al actualizar categoría de moderación: " << e.what() << std::endl;
    throw;
  }
}

void PostgresCategoriaModeracionRepository::remove(int id) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result check =
        txn.exec_params("SELECT COUNT(*) FROM categorias_moderacion WHERE id = $1", id);
    if (check[0][0].as<int>() == 0) {
      throw std::invalid_argument("Categoría de moderación no encontrada");
    }

    txn.exec_params("DELETE FROM categorias_moderacion WHERE id = $1", id);
    txn.commit();
  } catch (const std::exception& e) {
    std::cerr << "Error al eliminar categoría de moderación: " << e.what() << std::endl;
    throw;
  }
}

void PostgresCategoriaModeracionRepository::assignModerador(int categoriaId, int userId) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    pqxx::result check =
        txn.exec_params("SELECT COUNT(*) FROM categorias_moderacion WHERE id = $1", categoriaId);
    if (check[0][0].as<int>() == 0) {
      throw std::invalid_argument("Categoría de moderación no encontrada");
    }

    txn.exec_params(
        "INSERT INTO moderador_categorias (user_id, categoria_moderacion_id) "
        "VALUES ($1, $2) ON CONFLICT (user_id, categoria_moderacion_id) DO NOTHING",
        userId, categoriaId);
    txn.commit();
  } catch (const std::exception& e) {
    std::cerr << "Error al asignar moderador: " << e.what() << std::endl;
    throw;
  }
}

void PostgresCategoriaModeracionRepository::unassignModerador(int categoriaId, int userId) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    txn.exec_params(
        "DELETE FROM moderador_categorias WHERE categoria_moderacion_id = $1 AND user_id = $2",
        categoriaId, userId);
    txn.commit();
  } catch (const std::exception& e) {
    std::cerr << "Error al quitar moderador: " << e.what() << std::endl;
    throw;
  }
}

std::vector<int> PostgresCategoriaModeracionRepository::listModeradores(int categoriaId) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    const auto rows = txn.exec_params(
        "SELECT user_id FROM moderador_categorias WHERE categoria_moderacion_id = $1 "
        "ORDER BY user_id",
        categoriaId);

    std::vector<int> result;
    result.reserve(rows.size());
    for (const auto& row : rows) {
      result.push_back(row["user_id"].as<int>());
    }
    return result;
  } catch (const std::exception& e) {
    std::cerr << "Error al listar moderadores: " << e.what() << std::endl;
    throw;
  }
}

bool PostgresCategoriaModeracionRepository::isModeradorAssigned(int userId, int categoriaId) {
  try {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    const auto result = txn.exec_params(
        "SELECT COUNT(*) FROM moderador_categorias "
        "WHERE user_id = $1 AND categoria_moderacion_id = $2",
        userId, categoriaId);
    return result[0][0].as<int>() > 0;
  } catch (const std::exception& e) {
    std::cerr << "Error al verificar asignación de moderador: " << e.what() << std::endl;
    throw;
  }
}
