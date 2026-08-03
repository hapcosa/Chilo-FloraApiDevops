#ifndef POSTULACION_SERVICE_HPP
#define POSTULACION_SERVICE_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../repository/categoria_repository.hpp"
#include "../repository/postulacion_repository.hpp"

class PostulacionService {
private:
    std::shared_ptr<IPostulacionRepository> repository;
    std::shared_ptr<ICategoriaRepository> categorias;

public:
    PostulacionService(std::shared_ptr<IPostulacionRepository> repository,
                       std::shared_ptr<ICategoriaRepository> categorias);

    std::vector<PostulacionCurador> getPostulaciones(
        std::optional<PostulacionEstado> estado);
    std::vector<PostulacionCurador> getPostulacionesDe(int usuarioId);
    std::optional<PostulacionCurador> getPostulacionById(int id);

    PostulacionCurador postular(int usuarioId, const PostulacionCurador& postulacion);

    // Aprobar inserta la asignación en `moderador_categorias` dentro de la
    // misma transacción (ver IPostulacionRepository::aprobar).
    PostulacionCurador aprobar(int id, int revisadoPor);
    PostulacionCurador rechazar(int id, int revisadoPor, const std::string& motivo);
};

#endif // POSTULACION_SERVICE_HPP
