#include "../../include/services/moderacion_service.hpp"

#include <utility>

ModeracionService::ModeracionService(
    std::shared_ptr<IModeradorCategoriaRepository> repository)
    : repository(std::move(repository)) {}

bool ModeracionService::puedeEditarCategoria(int usuarioId,
                                             const std::string& rol,
                                             std::optional<int> categoriaId) {
    if (rol == "admin" || rol == "moderator") return true;
    if (!categoriaId) return false;
    return repository->esModeradorDe(usuarioId, *categoriaId);
}

std::vector<CategoriaModeracion> ModeracionService::categoriasDe(int usuarioId) {
    return repository->categoriasDe(usuarioId);
}

bool ModeracionService::asignarCurador(int usuarioId, int categoriaId, int asignadoPor) {
    return repository->asignar(usuarioId, categoriaId, asignadoPor);
}

bool ModeracionService::quitarCurador(int usuarioId, int categoriaId) {
    return repository->quitar(usuarioId, categoriaId);
}
