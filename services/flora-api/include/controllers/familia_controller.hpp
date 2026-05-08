#ifndef FAMILIA_CONTROLLER_HPP
#define FAMILIA_CONTROLLER_HPP

#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include "../services/familia_service.hpp"

class FamiliaController {
private:
    std::shared_ptr<FamiliaService> service;

    // Método para validar una Familia
    void validarFamilia(const Familia& Familia);

public:
    explicit FamiliaController(std::shared_ptr<FamiliaService> svc);

    // Manejadores de peticiones HTTP
    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void searchByNombre(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void update(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void remove(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void uploadImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void setImagenPrincipal(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void removeImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void serveImage(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getImagenesByFamilia(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router, std::shared_ptr<FamiliaController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/familias", Routes::bind(&FamiliaController::getAll, controller));
        Routes::Get(router, "/api/familias/:id", Routes::bind(&FamiliaController::getById, controller));
        Routes::Get(router, "/api/familias/search/nombre", Routes::bind(&FamiliaController::searchByNombre, controller));
        Routes::Post(router, "/api/familias", Routes::bind(&FamiliaController::create, controller));
        Routes::Put(router, "/api/familias/:id", Routes::bind(&FamiliaController::update, controller));
        Routes::Delete(router, "/api/familias/:id", Routes::bind(&FamiliaController::remove, controller));
        Routes::Post(router, "/api/familias/:id/images/:principal", Routes::bind(&FamiliaController::uploadImagen, controller));
        Routes::Put(router, "/api/familias/:id/images/:url", Routes::bind(&FamiliaController::setImagenPrincipal, controller));
        Routes::Get(router, "/api/familias/:id/images", Routes::bind(&FamiliaController::getImagenesByFamilia, controller));
        Routes::Get(router, "/api/images/familias/:filename", Routes::bind(&FamiliaController::serveImage, controller));
        Routes::Delete(router, "/api/images/familias/:filename", Routes::bind(&FamiliaController::removeImagen, controller));
}
};

#endif