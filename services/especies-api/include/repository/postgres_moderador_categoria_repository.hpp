#ifndef POSTGRES_MODERADOR_CATEGORIA_REPOSITORY_HPP
#define POSTGRES_MODERADOR_CATEGORIA_REPOSITORY_HPP

#include <memory>
#include <vector>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "moderador_categoria_repository.hpp"

class PostgresModeradorCategoriaRepository : public IModeradorCategoriaRepository {
private:
    std::shared_ptr<Database> database;

public:
    explicit PostgresModeradorCategoriaRepository(std::shared_ptr<Database> database);

    bool esModeradorDe(int usuarioId, int categoriaId) override;
    std::vector<CategoriaModeracion> categoriasDe(int usuarioId) override;
    bool asignar(int usuarioId, int categoriaId, int asignadoPor) override;
    bool quitar(int usuarioId, int categoriaId) override;
};

#endif // POSTGRES_MODERADOR_CATEGORIA_REPOSITORY_HPP
