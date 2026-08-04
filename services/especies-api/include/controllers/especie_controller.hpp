#ifndef ESPECIE_CONTROLLER_HPP
#define ESPECIE_CONTROLLER_HPP

#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include "../services/especie_service.hpp"
#include "../services/moderacion_service.hpp"
#include "../utils/constants.hpp"
#include "../utils/request_identity.hpp"

class EspecieController {
private:
    std::shared_ptr<EspecieService> service;
    std::shared_ptr<ModeracionService> moderacion;
    // Método para validar una especie
    void validarEspecie(const Especie& especie);
    // Exige sesión válida; si falta, ya envía el 401 y devuelve std::nullopt
    // para que el caller retorne.
    std::optional<RequestIdentity> requireSesion(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);
    // Exige curaduría sobre `categoriaId` (ADR #14): admin y moderator pasan
    // siempre; el resto necesita una fila en `moderador_categorias`. Envía el
    // 403 y devuelve false si no cumple.
    bool requireCuradorDeCategoria(const RequestIdentity& identity,
                                   std::optional<int> categoriaId,
                                   Pistache::Http::ResponseWriter& response);
    // Qué borradores puede ver quien hace la petición. Sin sesión, ninguno.
    EspecieVisibilidad visibilidadDe(const std::optional<RequestIdentity>& identity);
    // Cuerpo común de publicar/despublicar: mismo permiso, mismos errores.
    void cambiarEstado(const Pistache::Rest::Request& request,
                       Pistache::Http::ResponseWriter& response,
                       EspecieEstado destino);

public:
    EspecieController(std::shared_ptr<EspecieService> svc,
                      std::shared_ptr<ModeracionService> moderacion);

    // Manejadores de peticiones HTTP
    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void searchByNombreCientifico(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    // searchByGenero eliminado: usar GET /api/especies?genero_id=N
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void update(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void updateFotos(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void remove(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void publicar(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void despublicar(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void uploadImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void setImagenPrincipal(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void removeImagen(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void serveImage(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getImagenesByEspecie(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router, std::shared_ptr<EspecieController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/especies", Routes::bind(&EspecieController::getAll, controller));
        Routes::Get(router, "/api/v1/especies", Routes::bind(&EspecieController::getAll, controller));
        Routes::Get(router, "/api/especies/:id", Routes::bind(&EspecieController::getById, controller));
        Routes::Get(router, "/api/v1/especies/:id", Routes::bind(&EspecieController::getById, controller));
        Routes::Post(router, "/api/especies", Routes::bind(&EspecieController::create, controller));
        Routes::Post(router, "/api/v1/especies", Routes::bind(&EspecieController::create, controller));
        Routes::Put(router, "/api/especies/:id", Routes::bind(&EspecieController::update, controller));
        Routes::Put(router, "/api/v1/especies/:id", Routes::bind(&EspecieController::update, controller));
        Routes::Patch(router, "/api/especies/:id/fotos", Routes::bind(&EspecieController::updateFotos, controller));
        Routes::Patch(router, "/api/v1/especies/:id/fotos", Routes::bind(&EspecieController::updateFotos, controller));
        Routes::Delete(router, "/api/especies/:id", Routes::bind(&EspecieController::remove, controller));
        Routes::Delete(router, "/api/v1/especies/:id", Routes::bind(&EspecieController::remove, controller));
        Routes::Post(router, "/api/v1/especies/:id/publicar", Routes::bind(&EspecieController::publicar, controller));
        Routes::Post(router, "/api/v1/especies/:id/despublicar", Routes::bind(&EspecieController::despublicar, controller));
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
