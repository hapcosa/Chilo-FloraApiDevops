#ifndef INSIGNIA_CONTROLLER_HPP
#define INSIGNIA_CONTROLLER_HPP

#include <memory>
#include <optional>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/insignia_service.hpp"
#include "../utils/request_identity.hpp"

// Insignias de la Fase 9 (PR 11). Deliberadamente **no** hay endpoint de
// ranking: solo el catálogo y las insignias de una persona concreta.
class InsigniaController {
private:
    std::shared_ptr<InsigniaService> service;

    std::optional<RequestIdentity> requireSesion(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);
    std::optional<RequestIdentity> requireAdmin(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);

public:
    explicit InsigniaController(std::shared_ptr<InsigniaService> service);

    void getCatalogo(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getMias(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // Perfil público: las insignias de alguien son visibles para cualquier
    // sesión, igual que su nombre en el feed.
    void getDeUsuario(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // `?ids=1,2,3`. Existe para las pantallas que nombran a varias personas
    // —la lista de identificaciones de una ficha— y que si no harían una
    // petición por fila.
    void getDeUsuarios(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // Solo admin, e idempotente: es el job de las automáticas.
    void recalcular(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // Solo admin: {"usuario_id":1,"codigo":"curador","motivo":"Categoría Aves"}.
    void otorgar(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void revocar(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<InsigniaController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/v1/insignias",
                    Routes::bind(&InsigniaController::getCatalogo, controller));
        Routes::Get(router, "/api/v1/insignias/mias",
                    Routes::bind(&InsigniaController::getMias, controller));
        // Antes que la ruta con parámetro: `usuarios` es un segmento fijo y no
        // debe caer en `:usuarioId`.
        Routes::Get(router, "/api/v1/insignias/usuarios",
                    Routes::bind(&InsigniaController::getDeUsuarios, controller));
        Routes::Get(router, "/api/v1/insignias/usuario/:usuarioId",
                    Routes::bind(&InsigniaController::getDeUsuario, controller));
        Routes::Post(router, "/api/v1/insignias/recalcular",
                     Routes::bind(&InsigniaController::recalcular, controller));
        Routes::Post(router, "/api/v1/insignias/otorgar",
                     Routes::bind(&InsigniaController::otorgar, controller));
        Routes::Delete(router, "/api/v1/insignias/usuario/:usuarioId/:codigo",
                       Routes::bind(&InsigniaController::revocar, controller));
    }
};

#endif // INSIGNIA_CONTROLLER_HPP
