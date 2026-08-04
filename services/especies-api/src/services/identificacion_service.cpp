#include "../../include/services/identificacion_service.hpp"

#include <stdexcept>
#include <utility>

IdentificacionService::IdentificacionService(
    std::shared_ptr<IIdentificacionRepository> repository,
    std::shared_ptr<IAvistamientoRepository> avistamientos,
    std::shared_ptr<IEspecieRepository> especies,
    std::shared_ptr<ModeracionService> moderacion)
    : repository(std::move(repository)),
      avistamientos(std::move(avistamientos)),
      especies(std::move(especies)),
      moderacion(std::move(moderacion)) {}

std::vector<Identificacion> IdentificacionService::getIdentificaciones(
    int avistamientoId) {
    if (!avistamientos->findById(avistamientoId)) {
        throw std::out_of_range("avistamiento no encontrado");
    }
    return repository->findByAvistamiento(avistamientoId);
}

std::optional<Identificacion> IdentificacionService::getIdentificacionById(int id) {
    return repository->findById(id);
}

Identificacion IdentificacionService::identificar(
    int avistamientoId,
    int usuarioId,
    const std::string& rol,
    const Identificacion& identificacion) {
    const auto avistamiento = avistamientos->findById(avistamientoId);
    if (!avistamiento) {
        throw std::out_of_range("avistamiento no encontrado");
    }

    const auto especie = especies->findById(identificacion.getEspecieId());
    if (!especie) {
        throw std::out_of_range("especie no encontrada");
    }

    // Identificar como una especie de otro reino es casi siempre un error de
    // dedo en un formulario, y ensuciaría el conteo de coincidencias.
    if (especie->getReino() != avistamiento->getReino()) {
        throw std::invalid_argument(
            "la especie pertenece a otro reino que el avistamiento");
    }

    Identificacion nueva = identificacion;
    nueva.setAvistamientoId(avistamientoId);
    nueva.setUsuarioId(usuarioId);
    // El voto decisivo se resuelve aquí, contra la BD, y se persiste: quien
    // cura la categoría de esa especie cierra el avistamiento sin esperar
    // quórum (ADR #14). Aceptarlo del cuerpo sería dejar que cualquiera se
    // autoproclame curador.
    nueva.setDecisiva(
        moderacion->puedeEditarCategoria(usuarioId, rol, especie->getCategoriaId()));
    nueva.setRetirada(false);

    return repository->create(nueva, calcularGrado);
}

std::optional<Identificacion> IdentificacionService::retirar(int identificacionId) {
    return repository->retirar(identificacionId, calcularGrado);
}
