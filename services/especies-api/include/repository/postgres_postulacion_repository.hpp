#ifndef POSTGRES_POSTULACION_REPOSITORY_HPP
#define POSTGRES_POSTULACION_REPOSITORY_HPP

#include <memory>
#include <string>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "postulacion_repository.hpp"

class PostgresPostulacionRepository : public IPostulacionRepository {
private:
    std::shared_ptr<Database> database;

    PostulacionCurador mapRowToPostulacion(const pqxx::row& row);

public:
    explicit PostgresPostulacionRepository(std::shared_ptr<Database> database);

    std::vector<PostulacionCurador> findAll(
        std::optional<PostulacionEstado> estado) override;
    std::vector<PostulacionCurador> findByUsuario(int usuarioId) override;
    std::optional<PostulacionCurador> findById(int id) override;
    PostulacionCurador create(const PostulacionCurador& postulacion) override;
    PostulacionCurador aprobar(int id, int revisadoPor) override;
    PostulacionCurador rechazar(int id, int revisadoPor,
                                const std::string& motivo) override;
};

#endif // POSTGRES_POSTULACION_REPOSITORY_HPP
