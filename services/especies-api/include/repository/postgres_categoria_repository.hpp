#ifndef POSTGRES_CATEGORIA_REPOSITORY_HPP
#define POSTGRES_CATEGORIA_REPOSITORY_HPP

#include <memory>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "categoria_repository.hpp"

class PostgresCategoriaRepository : public ICategoriaRepository {
private:
    std::shared_ptr<Database> database;

    CategoriaModeracion mapRowToCategoria(const pqxx::row& row);

public:
    explicit PostgresCategoriaRepository(std::shared_ptr<Database> database);

    std::vector<CategoriaModeracion> findAll(std::optional<Reino> reino) override;
    std::optional<CategoriaModeracion> findById(int id) override;
    CategoriaModeracion create(const CategoriaModeracion& categoria) override;
    CategoriaModeracion update(int id, const CategoriaModeracion& categoria) override;
    void remove(int id) override;
};

#endif // POSTGRES_CATEGORIA_REPOSITORY_HPP
