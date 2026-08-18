#ifndef AREA_PROTEGIDA_CONTROLLER_HPP
#define AREA_PROTEGIDA_CONTROLLER_HPP

#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/area_protegida_service.hpp"

// Parques y áreas protegidas de Chiloé. Todo público: es información de
// divulgación, y el gancho para quien visita la isla.
class AreaProtegidaController {
private:
    std::shared_ptr<AreaProtegidaService> service;

public:
    explicit AreaProtegidaController(std::shared_ptr<AreaProtegidaService> service);

    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getEspecies(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<AreaProtegidaController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/v1/areas-protegidas",
                    Routes::bind(&AreaProtegidaController::getAll, controller));
        Routes::Get(router, "/api/v1/areas-protegidas/:id",
                    Routes::bind(&AreaProtegidaController::getById, controller));
        Routes::Get(router, "/api/v1/areas-protegidas/:id/especies",
                    Routes::bind(&AreaProtegidaController::getEspecies, controller));
    }
};

#endif // AREA_PROTEGIDA_CONTROLLER_HPP
