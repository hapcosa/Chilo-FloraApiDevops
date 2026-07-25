#ifndef POSTGRES_CATEGORIA_MODERACION_REPOSITORY_HPP
#define POSTGRES_CATEGORIA_MODERACION_REPOSITORY_HPP

#include <memory>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "categoria_moderacion_repository.hpp"

class PostgresCategoriaModeracionRepository : public ICategoriaModeracionRepository {
private:
    std::shared_ptr<Database> database;

    CategoriaModeracion mapRowToCategoria(const pqxx::row& row);

public:
    explicit PostgresCategoriaModeracionRepository(std::shared_ptr<Database> database);

    CategoriaModeracion create(const CategoriaModeracion& categoria) override;
    std::vector<CategoriaModeracion> findAll() override;
    std::optional<CategoriaModeracion> findById(int id) override;
    CategoriaModeracion update(const CategoriaModeracion& categoria) override;
    void remove(int id) override;

    void assignModerador(int categoriaId, int userId) override;
    void unassignModerador(int categoriaId, int userId) override;
    std::vector<int> listModeradores(int categoriaId) override;
    bool isModeradorAssigned(int userId, int categoriaId) override;
};

#endif  // POSTGRES_CATEGORIA_MODERACION_REPOSITORY_HPP
