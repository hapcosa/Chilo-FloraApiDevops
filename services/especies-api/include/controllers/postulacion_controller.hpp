#ifndef POSTULACION_CONTROLLER_HPP
#define POSTULACION_CONTROLLER_HPP

#include <memory>
#include <optional>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/postulacion_service.hpp"
#include "../utils/request_identity.hpp"

// Postularse a curador de una categoría y resolución por parte de un admin.
class PostulacionController {
private:
    std::shared_ptr<PostulacionService> service;

    std::optional<RequestIdentity> requireSesion(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);
    std::optional<RequestIdentity> requireAdmin(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);

public:
    explicit PostulacionController(std::shared_ptr<PostulacionService> service);

    // Cualquier sesión: crea la postulación a nombre del usuario verificado.
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // Admin ve la bandeja completa; el resto solo las suyas.
    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // Solo admin: {"estado":"aprobada"} o {"estado":"rechazada","motivo":"..."}.
    void resolver(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<PostulacionController> controller) {
        using namespace Pistache::Rest;

        Routes::Post(router, "/api/v1/postulaciones",
                     Routes::bind(&PostulacionController::create, controller));
        Routes::Get(router, "/api/v1/postulaciones",
                    Routes::bind(&PostulacionController::getAll, controller));
        Routes::Get(router, "/api/v1/postulaciones/:id",
                    Routes::bind(&PostulacionController::getById, controller));
        Routes::Patch(router, "/api/v1/postulaciones/:id",
                      Routes::bind(&PostulacionController::resolver, controller));
    }
};

#endif // POSTULACION_CONTROLLER_HPP
