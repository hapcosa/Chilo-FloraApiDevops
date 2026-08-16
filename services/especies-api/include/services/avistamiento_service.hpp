#ifndef AVISTAMIENTO_SERVICE_HPP
#define AVISTAMIENTO_SERVICE_HPP

#include <memory>
#include <optional>

#include "../repository/avistamiento_repository.hpp"
#include "upload_presign_service.hpp"

class AvistamientoService {
private:
    std::shared_ptr<IAvistamientoRepository> repository;
    std::shared_ptr<UploadPresignService> storageService;

    void validateAvistamiento(const Avistamiento& avistamiento) const;
    void resolverFotoUrl(Avistamiento& avistamiento) const;

public:
    AvistamientoService(std::shared_ptr<IAvistamientoRepository> repository,
                        std::shared_ptr<UploadPresignService> storageService);

    Avistamiento createAvistamiento(const Avistamiento& avistamiento);
    AvistamientoSearchResult searchAvistamientos(const AvistamientoFilters& filters);
    std::optional<Avistamiento> getAvistamientoById(int id);
    Avistamiento moderateAvistamiento(int id, const ModeracionAvistamiento& moderacion);
    std::optional<Avistamiento> compartirAvistamiento(int id, int usuarioId);
};

#endif // AVISTAMIENTO_SERVICE_HPP

