#ifndef ESPECIE_CONTROLLER_HPP
#define ESPECIE_CONTROLLER_HPP

#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include "../services/especie_service.hpp"
#include "../utils/constants.hpp"

class EspecieController {
private:
    std::shared_ptr<EspecieService> service;
    // Método para validar una especie
    void validarEspecie(const Especie& especie);

public:
    explicit EspecieController(std::shared_ptr<EspecieService> svc);

    // Manejadores de peticiones HTTP
    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void searchByNombreCientifico(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // searchByGenero eliminado: usar GET /api/especies?genero_id=N
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void update(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void updateFotos(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void remove(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void uploadImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void setImagenPrincipal(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void removeImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void serveImage(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getImagenesByEspecie(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router, std::shared_ptr<EspecieController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/especies", Routes::bind(&EspecieController::getAll, controller));
        Routes::Get(router, "/api/especies/:id", Routes::bind(&EspecieController::getById, controller));
        Routes::Post(router, "/api/especies", Routes::bind(&EspecieController::create, controller));
        Routes::Patch(router, "/api/v1/especies/:id/fotos", Routes::bind(&EspecieController::updateFotos, controller));
        Routes::Patch(router, "/api/especies/:id/fotos", Routes::bind(&EspecieController::updateFotos, controller));
        Routes::Put(router, "/api/especies/:id", Routes::bind(&EspecieController::update, controller));
        Routes::Delete(router, "/api/especies/:id", Routes::bind(&EspecieController::remove, controller));
        Routes::Post(router, "/api/especies/:id/images", Routes::bind(&EspecieController::uploadImagen, controller));
        Routes::Get(router, "/api/especies/search/nombre", Routes::bind(&EspecieController::searchByNombreCientifico, controller));
        Routes::Post(router, "/api/especies/:id/images/:principal", Routes::bind(&EspecieController::uploadImagen, controller));
        Routes::Put(router, "/api/especies/:id/images/:url", Routes::bind(&EspecieController::setImagenPrincipal, controller));
        Routes::Get(router, "/api/especies/:id/images", Routes::bind(&EspecieController::getImagenesByEspecie, controller));
        Routes::Get(router, "/api/images/especies/:filename", Routes::bind(&EspecieController::serveImage, controller));
        Routes::Delete(router, "/api/images/especies/:filename", Routes::bind(&EspecieController::removeImagen, controller));
}
};

#endif // ESPECIE_CONTROLLER_HPP
