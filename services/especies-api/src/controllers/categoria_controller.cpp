#include "../../include/controllers/categoria_controller.hpp"
#include "../../include/utils/query_params.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <utility>

using json = nlohmann::json;

namespace {

void sendJson(Pistache::Http::ResponseWriter& response,
              Pistache::Http::Code code,
              const json& payload) {
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(code, payload.dump());
}

} // namespace

CategoriaController::CategoriaController(std::shared_ptr<CategoriaService> service)
    : service(std::move(service)) {}

std::optional<RequestIdentity> CategoriaController::requireAdmin(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter& response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false},
                  {"error", "No se pudo verificar la sesión del usuario"}});
        return std::nullopt;
    }
    if (!identity->isAdmin()) {
        sendJson(response, Pistache::Http::Code::Forbidden,
                 {{"success", false}, {"error", "Se requiere rol admin"}});
        return std::nullopt;
    }
    return identity;
}

void CategoriaController::getAll(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    try {
        std::optional<Reino> reino;
        const auto query = request.query();
        if (query.has("reino")) {
            reino = reinoFromString(utils::percentDecode(query.get("reino").value()));
        }

        json data = json::array();
        for (const auto& categoria : service->getCategorias(reino)) {
            data.push_back(categoria.toJson());
        }

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", data}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void CategoriaController::getById(const Pistache::Rest::Request& request,
                                  Pistache::Http::ResponseWriter response) {
    try {
        const int id = request.param(":id").as<int>();
        const auto categoria = service->getCategoriaById(id);
        if (!categoria) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "categoría no encontrada"}});
            return;
        }

        sendJson(response, Pistache::Http::Code::Ok, categoria->toJson());
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void CategoriaController::create(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    if (!requireAdmin(request, response)) return;

    try {
        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const auto payload = json::parse(request.body());
        const auto categoria =
            service->createCategoria(CategoriaModeracion::fromJson(payload));

        sendJson(response, Pistache::Http::Code::Created, categoria.toJson());
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false},
                  {"error", "JSON inválido: " + std::string(error.what())}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void CategoriaController::update(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    if (!requireAdmin(request, response)) return;

    try {
        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const int id = request.param(":id").as<int>();
        const auto payload = json::parse(request.body());
        const auto categoria =
            service->updateCategoria(id, CategoriaModeracion::fromJson(payload));

        sendJson(response, Pistache::Http::Code::Ok, categoria.toJson());
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false},
                  {"error", "JSON inválido: " + std::string(error.what())}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::out_of_range& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void CategoriaController::remove(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    if (!requireAdmin(request, response)) return;

    try {
        const int id = request.param(":id").as<int>();
        service->deleteCategoria(id);

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"message", "categoría eliminada"}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Conflict,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::out_of_range& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}
