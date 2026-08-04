#ifndef IDENTIFICACION_CONTROLLER_HPP
#define IDENTIFICACION_CONTROLLER_HPP

#include <memory>
#include <optional>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/identificacion_service.hpp"
#include "../utils/request_identity.hpp"

// Identificación comunitaria de avistamientos. Cuelga de
// /api/v1/avistamientos/:id porque una identificación no existe sin el
// avistamiento que identifica.
class IdentificacionController {
private:
    std::shared_ptr<IdentificacionService> service;

    std::optional<RequestIdentity> requireSesion(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);

public:
    explicit IdentificacionController(std::shared_ptr<IdentificacionService> service);

    // Cualquier sesión puede identificar: es el punto del sistema.
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // Solo el autor o un admin; retirar marca `retirada`, no borra la fila.
    void retirar(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<IdentificacionController> controller) {
        using namespace Pistache::Rest;

        Routes::Post(router, "/api/v1/avistamientos/:id/identificaciones",
                     Routes::bind(&IdentificacionController::create, controller));
        Routes::Get(router, "/api/v1/avistamientos/:id/identificaciones",
                    Routes::bind(&IdentificacionController::getAll, controller));
        Routes::Delete(router,
                       "/api/v1/avistamientos/:id/identificaciones/:idIdentificacion",
                       Routes::bind(&IdentificacionController::retirar, controller));
    }
};

#endif // IDENTIFICACION_CONTROLLER_HPP
