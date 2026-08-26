#include "../../include/repository/postgres_categoria_repository.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "../../include/utils/timestamps.hpp"

namespace {

constexpr const char* kSelectCols =
    "id, slug, nombre, reino, descripcion, created_at, updated_at";

// El listado agrega cuántas fichas publicadas cuelgan de cada categoría: la app
// esconde los subgrupos vacíos en vez de ofrecer un filtro que no devuelve
// nada. Los borradores no cuentan, porque para quien navega no existen.
constexpr const char* kSelectColsConTotal =
    "c.id, c.slug, c.nombre, c.reino, c.descripcion, c.created_at, c.updated_at,"
    " COUNT(e.id) FILTER (WHERE e.estado = 'publicada') AS total_especies";

std::optional<std::string> optStr(const pqxx::field& field) {
    if (field.is_null()) return std::nullopt;
    return std::string(field.c_str());
}

std::string quoteOptString(pqxx::work& txn, const std::optional<std::string>& value) {
    return value ? txn.quote(*value) : "NULL";
}

} // namespace

PostgresCategoriaRepository::PostgresCategoriaRepository(std::shared_ptr<Database> database)
    : database(std::move(database)) {}

CategoriaModeracion PostgresCategoriaRepository::mapRowToCategoria(const pqxx::row& row) {
    CategoriaModeracion categoria;
    categoria.setId(row["id"].as<int>());
    categoria.setSlug(row["slug"].c_str());
    categoria.setNombre(row["nombre"].c_str());
    categoria.setReino(reinoFromString(row["reino"].c_str()));
    categoria.setDescripcion(optStr(row["descripcion"]));
    categoria.setCreatedAt(utils::toIso8601Opt(optStr(row["created_at"])));
    categoria.setUpdatedAt(utils::toIso8601Opt(optStr(row["updated_at"])));
    return categoria;
}

std::vector<CategoriaModeracion> PostgresCategoriaRepository::findAll(
    std::optional<Reino> reino) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        std::string sql = std::string("SELECT ") + kSelectColsConTotal
            + " FROM categorias_moderacion c"
              " LEFT JOIN especies e ON e.categoria_id = c.id";
        if (reino) {
            sql += " WHERE c.reino = " + txn.quote(reinoToString(*reino)) + "::reino_enum";
        }
        sql += " GROUP BY c.id ORDER BY c.reino, c.nombre";

        const auto rows = txn.exec(sql);

        std::vector<CategoriaModeracion> categorias;
        categorias.reserve(rows.size());
        for (const auto& row : rows) {
            auto categoria = mapRowToCategoria(row);
            categoria.setTotalEspecies(row["total_especies"].as<int>());
            categorias.push_back(std::move(categoria));
        }
        return categorias;
    } catch (const std::exception& error) {
        std::cerr << "Error al listar categorías: " << error.what() << std::endl;
        throw;
    }
}

std::optional<CategoriaModeracion> PostgresCategoriaRepository::findById(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto result = txn.exec_params(
            std::string("SELECT ") + kSelectCols
                + " FROM categorias_moderacion WHERE id = $1",
            id);

        if (result.empty()) return std::nullopt;
        return mapRowToCategoria(result[0]);
    } catch (const std::exception& error) {
        std::cerr << "Error al obtener categoría: " << error.what() << std::endl;
        throw;
    }
}

CategoriaModeracion PostgresCategoriaRepository::create(
    const CategoriaModeracion& categoria) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const std::string sql =
            std::string("INSERT INTO categorias_moderacion (slug, nombre, reino, descripcion)")
            + " VALUES ("
            + txn.quote(categoria.getSlug()) + ", "
            + txn.quote(categoria.getNombre()) + ", "
            + txn.quote(reinoToString(categoria.getReino())) + "::reino_enum, "
            + quoteOptString(txn, categoria.getDescripcion())
            + ") RETURNING " + kSelectCols;

        const auto result = txn.exec(sql);
        txn.commit();
        return mapRowToCategoria(result[0]);
    } catch (const pqxx::unique_violation&) {
        throw std::invalid_argument("ya existe una categoría con ese slug");
    } catch (const std::exception& error) {
        std::cerr << "Error al crear categoría: " << error.what() << std::endl;
        throw;
    }
}

CategoriaModeracion PostgresCategoriaRepository::update(
    int id,
    const CategoriaModeracion& categoria) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        const std::string sql =
            std::string("UPDATE categorias_moderacion SET ")
            + "slug = " + txn.quote(categoria.getSlug()) + ", "
            + "nombre = " + txn.quote(categoria.getNombre()) + ", "
            + "reino = " + txn.quote(reinoToString(categoria.getReino())) + "::reino_enum, "
            + "descripcion = " + quoteOptString(txn, categoria.getDescripcion())
            + " WHERE id = " + txn.quote(id)
            + " RETURNING " + kSelectCols;

        const auto result = txn.exec(sql);
        if (result.empty()) {
            throw std::out_of_range("categoría no encontrada");
        }

        txn.commit();
        return mapRowToCategoria(result[0]);
    } catch (const pqxx::unique_violation&) {
        throw std::invalid_argument("ya existe una categoría con ese slug");
    } catch (const std::exception& error) {
        std::cerr << "Error al actualizar categoría: " << error.what() << std::endl;
        throw;
    }
}

void PostgresCategoriaRepository::remove(int id) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);
        const auto result = txn.exec_params(
            "DELETE FROM categorias_moderacion WHERE id = $1 RETURNING id", id);

        if (result.empty()) {
            throw std::out_of_range("categoría no encontrada");
        }

        txn.commit();
    } catch (const pqxx::foreign_key_violation&) {
        // ON DELETE RESTRICT desde especies.categoria_id: borrar dejaría fichas
        // sin clasificar y por tanto sin nadie autorizado a editarlas.
        throw std::invalid_argument("la categoría tiene especies asignadas");
    } catch (const std::exception& error) {
        std::cerr << "Error al borrar categoría: " << error.what() << std::endl;
        throw;
    }
}
