#include "../../include/services/avistamiento_service.hpp"

#include <stdexcept>
#include <utility>

namespace {

constexpr const char* AvistamientosBucket = "avistamientos-fotos";

} // namespace

AvistamientoService::AvistamientoService(
    std::shared_ptr<IAvistamientoRepository> repository,
    std::shared_ptr<UploadPresignService> storageService)
    : repository(std::move(repository)), storageService(std::move(storageService)) {}

void AvistamientoService::validateAvistamiento(const Avistamiento& avistamiento) const {
    if (!avistamiento.esValido()) {
        throw std::invalid_argument("avistamiento inválido");
    }

    if (!storageService) {
        throw std::runtime_error("storage service no configurado");
    }

    if (!storageService->objectExists(AvistamientosBucket, avistamiento.getFotoKey())) {
        throw std::invalid_argument("foto_key no existe en avistamientos-fotos");
    }
}

Avistamiento AvistamientoService::createAvistamiento(const Avistamiento& avistamiento) {
    validateAvistamiento(avistamiento);
    return repository->create(avistamiento);
}

AvistamientoSearchResult AvistamientoService::searchAvistamientos(
    const AvistamientoFilters& filters) {
    return repository->find(filters);
}

std::optional<Avistamiento> AvistamientoService::getAvistamientoById(int id) {
    return repository->findById(id);
}

Avistamiento AvistamientoService::moderateAvistamiento(
    int id,
    const ModeracionAvistamiento& moderacion) {
    if (moderacion.estado == AvistamientoEstado::Pendiente) {
        throw std::invalid_argument("la moderación debe aprobar o rechazar");
    }
    if (moderacion.estado == AvistamientoEstado::Rechazado &&
        (!moderacion.motivo_rechazo || moderacion.motivo_rechazo->empty())) {
        throw std::invalid_argument("motivo_rechazo es obligatorio al rechazar");
    }

    return repository->moderate(id, moderacion);
}

