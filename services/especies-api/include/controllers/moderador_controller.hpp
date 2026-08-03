#ifndef MODERADOR_CONTROLLER_HPP
#define MODERADOR_CONTROLLER_HPP

#include <memory>
#include <optional>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/moderacion_service.hpp"
#include "../utils/request_identity.hpp"

// Alta y baja de curadores de categoría. Mientras no exista el flujo de
// postulaciones (PR C) esta es la única vía para nombrar curadores.
class ModeradorController {
private:
    std::shared_ptr<ModeracionService> service;

    std::optional<RequestIdentity> requireAdmin(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);

public:
    explicit ModeradorController(std::shared_ptr<ModeracionService> service);

    void asignar(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void quitar(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void categoriasDeUsuario(const Pistache::Rest::Request& request,
                             Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<ModeradorController> controller) {
        using namespace Pistache::Rest;

        Routes::Post(router, "/api/v1/categorias/:id/moderadores/:usuarioId",
                     Routes::bind(&ModeradorController::asignar, controller));
        Routes::Delete(router, "/api/v1/categorias/:id/moderadores/:usuarioId",
                       Routes::bind(&ModeradorController::quitar, controller));
        Routes::Get(router, "/api/v1/moderadores/:usuarioId/categorias",
                    Routes::bind(&ModeradorController::categoriasDeUsuario, controller));
    }
};

#endif // MODERADOR_CONTROLLER_HPP
