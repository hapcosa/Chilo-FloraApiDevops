#ifndef CATEGORIA_MODERACION_CONTROLLER_HPP
#define CATEGORIA_MODERACION_CONTROLLER_HPP

#include <memory>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/categoria_moderacion_service.hpp"

// Gestión de categorías de moderación y su asignación muchos-a-muchos con
// moderadores. Todo endpoint aquí es admin-only (ver requireAdmin en
// utils/request_identity.hpp): la gestión de quién modera qué es una
// decisión administrativa, no algo que un moderador se auto-asigne.
class CategoriaModeracionController {
private:
    std::shared_ptr<CategoriaModeracionService> service;

public:
    explicit CategoriaModeracionController(std::shared_ptr<CategoriaModeracionService> svc);

    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void update(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void remove(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    void listModeradores(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void assignModerador(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void unassignModerador(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                             std::shared_ptr<CategoriaModeracionController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/v1/categorias-moderacion",
                    Routes::bind(&CategoriaModeracionController::getAll, controller));
        Routes::Post(router, "/api/v1/categorias-moderacion",
                     Routes::bind(&CategoriaModeracionController::create, controller));
        Routes::Get(router, "/api/v1/categorias-moderacion/:id",
                    Routes::bind(&CategoriaModeracionController::getById, controller));
        Routes::Put(router, "/api/v1/categorias-moderacion/:id",
                    Routes::bind(&CategoriaModeracionController::update, controller));
        Routes::Delete(router, "/api/v1/categorias-moderacion/:id",
                       Routes::bind(&CategoriaModeracionController::remove, controller));
        Routes::Get(router, "/api/v1/categorias-moderacion/:id/moderadores",
                    Routes::bind(&CategoriaModeracionController::listModeradores, controller));
        Routes::Post(router, "/api/v1/categorias-moderacion/:id/moderadores/:user_id",
                     Routes::bind(&CategoriaModeracionController::assignModerador, controller));
        Routes::Delete(router, "/api/v1/categorias-moderacion/:id/moderadores/:user_id",
                       Routes::bind(&CategoriaModeracionController::unassignModerador, controller));
    }
};

#endif  // CATEGORIA_MODERACION_CONTROLLER_HPP
