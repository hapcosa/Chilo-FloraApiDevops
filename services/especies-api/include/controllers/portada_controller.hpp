#ifndef PORTADA_CONTROLLER_HPP
#define PORTADA_CONTROLLER_HPP

#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/portada_service.hpp"

// La pantalla de inicio de la app en una sola llamada.
//
// Público y sin autenticación, como el resto de la divulgación. El servicio ya
// recorta a fichas publicadas y encuentros aprobados y públicos, así que aquí
// no hay ninguna decisión de visibilidad que tomar.
class PortadaController {
private:
    std::shared_ptr<PortadaService> service;

public:
    explicit PortadaController(std::shared_ptr<PortadaService> service);

    void getPortada(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<PortadaController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/v1/portada",
                    Routes::bind(&PortadaController::getPortada, controller));
    }
};

#endif // PORTADA_CONTROLLER_HPP
