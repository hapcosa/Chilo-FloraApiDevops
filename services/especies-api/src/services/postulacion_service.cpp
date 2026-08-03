#include "../../include/services/postulacion_service.hpp"

#include <stdexcept>
#include <utility>

PostulacionService::PostulacionService(
    std::shared_ptr<IPostulacionRepository> repository,
    std::shared_ptr<ICategoriaRepository> categorias)
    : repository(std::move(repository)), categorias(std::move(categorias)) {}

std::vector<PostulacionCurador> PostulacionService::getPostulaciones(
    std::optional<PostulacionEstado> estado) {
    return repository->findAll(estado);
}

std::vector<PostulacionCurador> PostulacionService::getPostulacionesDe(int usuarioId) {
    return repository->findByUsuario(usuarioId);
}

std::optional<PostulacionCurador> PostulacionService::getPostulacionById(int id) {
    return repository->findById(id);
}

PostulacionCurador PostulacionService::postular(
    int usuarioId, const PostulacionCurador& postulacion) {
    PostulacionCurador nueva = postulacion;
    // El postulante sale de la identidad verificada, nunca del cuerpo: nadie
    // postula en nombre de otro.
    nueva.setUsuarioId(usuarioId);
    nueva.setEstado(PostulacionEstado::Pendiente);

    if (!nueva.esValida()) {
        throw std::invalid_argument("los datos de la postulación no son válidos");
    }

    // La FK ya lo garantizaría, pero un 404 explícito distingue "esa categoría
    // no existe" de un fallo de integridad cualquiera.
    if (!categorias->findById(nueva.getCategoriaId())) {
        throw std::out_of_range("categoría no encontrada");
    }

    return repository->create(nueva);
}

PostulacionCurador PostulacionService::aprobar(int id, int revisadoPor) {
    return repository->aprobar(id, revisadoPor);
}

PostulacionCurador PostulacionService::rechazar(int id,
                                                int revisadoPor,
                                                const std::string& motivo) {
    if (motivo.empty()) {
        // La restricción CHECK de la tabla lo rechazaría igual, pero como
        // error de aplicación el mensaje es útil para el admin.
        throw std::invalid_argument("rechazar exige un 'motivo'");
    }
    return repository->rechazar(id, revisadoPor, motivo);
}
