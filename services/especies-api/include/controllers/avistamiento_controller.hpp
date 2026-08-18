#ifndef AVISTAMIENTO_CONTROLLER_HPP
#define AVISTAMIENTO_CONTROLLER_HPP

#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/avistamiento_service.hpp"

class AvistamientoController {
private:
    std::shared_ptr<AvistamientoService> service;

public:
    explicit AvistamientoController(std::shared_ptr<AvistamientoService> service);

    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getMapa(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void moderate(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void compartir(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<AvistamientoController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/v1/avistamientos",
                    Routes::bind(&AvistamientoController::getAll, controller));
        // Antes que "/:id": si no, el router trataría "mapa" como un id y el
        // endpoint respondería 400 por un parámetro que nadie mandó.
        Routes::Get(router, "/api/v1/avistamientos/mapa",
                    Routes::bind(&AvistamientoController::getMapa, controller));
        Routes::Get(router, "/api/v1/avistamientos/:id",
                    Routes::bind(&AvistamientoController::getById, controller));
        Routes::Post(router, "/api/v1/avistamientos",
                     Routes::bind(&AvistamientoController::create, controller));
        Routes::Patch(router, "/api/v1/avistamientos/:id/moderacion",
                      Routes::bind(&AvistamientoController::moderate, controller));
        Routes::Patch(router, "/api/v1/avistamientos/:id/compartir",
                      Routes::bind(&AvistamientoController::compartir, controller));
    }
};

#endif // AVISTAMIENTO_CONTROLLER_HPP

