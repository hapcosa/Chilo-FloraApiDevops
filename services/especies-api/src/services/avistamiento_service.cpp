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

// La foto de un encuentro vive en un bucket privado, así que la única forma de
// que un cliente la muestre es una URL firmada de corta duración. Se resuelve al
// responder y no se guarda: caduca. Un fallo al firmar no tumba la respuesta —
// una tarjeta sin foto es peor que nada, pero mucho menos que un 500.
void AvistamientoService::resolverFotoUrl(Avistamiento& avistamiento) const {
    if (!storageService || avistamiento.getFotoKey().empty()) {
        return;
    }
    try {
        avistamiento.setFotoUrl(
            storageService->createPresignedGet(AvistamientosBucket,
                                               avistamiento.getFotoKey()));
    } catch (const std::exception&) {
        avistamiento.setFotoUrl(std::nullopt);
    }
}

Avistamiento AvistamientoService::createAvistamiento(const Avistamiento& avistamiento) {
    validateAvistamiento(avistamiento);
    auto creado = repository->create(avistamiento);
    resolverFotoUrl(creado);
    return creado;
}

AvistamientoSearchResult AvistamientoService::searchAvistamientos(
    const AvistamientoFilters& filters) {
    auto result = repository->find(filters);
    for (auto& avistamiento : result.data) {
        resolverFotoUrl(avistamiento);
    }
    return result;
}

std::optional<Avistamiento> AvistamientoService::getAvistamientoById(int id) {
    auto avistamiento = repository->findById(id);
    if (avistamiento) {
        resolverFotoUrl(*avistamiento);
    }
    return avistamiento;
}

std::optional<Avistamiento> AvistamientoService::compartirAvistamiento(int id, int usuarioId) {
    auto avistamiento = repository->compartir(id, usuarioId);
    if (avistamiento) {
        resolverFotoUrl(*avistamiento);
    }
    return avistamiento;
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

    auto moderado = repository->moderate(id, moderacion);
    resolverFotoUrl(moderado);
    return moderado;
}

