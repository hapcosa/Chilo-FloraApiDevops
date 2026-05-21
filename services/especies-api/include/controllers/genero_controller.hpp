#ifndef GENERO_CONTROLLER_HPP
#define GENERO_CONTROLLER_HPP

#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include "../services/genero_service.hpp"
#include "../utils/constants.hpp"
class GeneroController {
private:
    std::shared_ptr<GeneroService> service;

    // Método para validar una Genero
    void validarGenero(const Genero& Genero);

public:
    explicit GeneroController(std::shared_ptr<GeneroService> svc);

    // Manejadores de peticiones HTTP
    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void update(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void remove(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    void uploadImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void setImagenPrincipal(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void removeImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void serveImage(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getImagenesByGenero(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);


    static void setupRoutes(Pistache::Rest::Router& router, std::shared_ptr<GeneroController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/generos", Routes::bind(&GeneroController::getAll, controller));
        Routes::Get(router, "/api/generos/:id", Routes::bind(&GeneroController::getById, controller));
        Routes::Post(router, "/api/generos", Routes::bind(&GeneroController::create, controller));
        Routes::Put(router, "/api/generos/:id", Routes::bind(&GeneroController::update, controller));
        Routes::Delete(router, "/api/generos/:id", Routes::bind(&GeneroController::remove, controller));
        Routes::Post(router, "/api/generos/:id/images/:principal", Routes::bind(&GeneroController::uploadImagen, controller));
        Routes::Put(router, "/api/generos/:id/images/:url", Routes::bind(&GeneroController::setImagenPrincipal, controller));
        Routes::Get(router, "/api/generos/:id/images", Routes::bind(&GeneroController::getImagenesByGenero, controller));
        Routes::Get(router, "/api/images/generos/:filename", Routes::bind(&GeneroController::serveImage, controller));
        Routes::Delete(router, "/api/images/generos/:filename", Routes::bind(&GeneroController::removeImagen, controller));
    }
};

#endif
