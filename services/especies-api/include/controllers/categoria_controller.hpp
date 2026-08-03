#ifndef CATEGORIA_CONTROLLER_HPP
#define CATEGORIA_CONTROLLER_HPP

#include <memory>
#include <optional>
#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/categoria_service.hpp"
#include "../utils/request_identity.hpp"

class CategoriaController {
private:
    std::shared_ptr<CategoriaService> service;

    // El catálogo de categorías define quién puede editar qué, así que
    // mantenerlo es privilegio de admin: un curador no puede fabricarse una
    // categoría a medida.
    std::optional<RequestIdentity> requireAdmin(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response);

public:
    explicit CategoriaController(std::shared_ptr<CategoriaService> service);

    void getAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getById(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void create(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void update(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void remove(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<CategoriaController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/v1/categorias",
                    Routes::bind(&CategoriaController::getAll, controller));
        Routes::Get(router, "/api/v1/categorias/:id",
                    Routes::bind(&CategoriaController::getById, controller));
        Routes::Post(router, "/api/v1/categorias",
                     Routes::bind(&CategoriaController::create, controller));
        Routes::Put(router, "/api/v1/categorias/:id",
                    Routes::bind(&CategoriaController::update, controller));
        Routes::Delete(router, "/api/v1/categorias/:id",
                       Routes::bind(&CategoriaController::remove, controller));
    }
};

#endif // CATEGORIA_CONTROLLER_HPP
