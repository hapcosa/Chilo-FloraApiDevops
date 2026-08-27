#include "../../include/controllers/insignia_controller.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using json = nlohmann::json;

namespace {

void sendJson(Pistache::Http::ResponseWriter& response,
              Pistache::Http::Code code,
              const json& payload) {
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(code, payload.dump());
}

json toArray(const std::vector<InsigniaOtorgada>& otorgadas) {
    json data = json::array();
    for (const auto& otorgada : otorgadas) {
        data.push_back(otorgada.toJson());
    }
    return data;
}

} // namespace

InsigniaController::InsigniaController(std::shared_ptr<InsigniaService> service)
    : service(std::move(service)) {}

std::optional<RequestIdentity> InsigniaController::requireSesion(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter& response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false},
                  {"error", "No se pudo verificar la sesión del usuario"}});
        return std::nullopt;
    }
    return identity;
}

std::optional<RequestIdentity> InsigniaController::requireAdmin(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter& response) {
    auto identity = requireSesion(request, response);
    if (!identity) return std::nullopt;

    if (!identity->isAdmin()) {
        sendJson(response, Pistache::Http::Code::Forbidden,
                 {{"success", false}, {"error", "Se requiere rol admin"}});
        return std::nullopt;
    }
    return identity;
}

void InsigniaController::getCatalogo(const Pistache::Rest::Request& request,
                                     Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        json data = json::array();
        for (const auto& insignia : service->getCatalogo()) {
            data.push_back(insignia.toJson());
        }
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", data}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void InsigniaController::getMias(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true},
                  {"data", toArray(service->getInsigniasDe(identity->userId))}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void InsigniaController::getDeUsuario(const Pistache::Rest::Request& request,
                                      Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        const int usuarioId = request.param(":usuarioId").as<int>();
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true},
                  {"data", toArray(service->getInsigniasDe(usuarioId))}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void InsigniaController::recalcular(const Pistache::Rest::Request& request,
                                    Pistache::Http::ResponseWriter response) {
    auto identity = requireAdmin(request, response);
    if (!identity) return;

    try {
        const int otorgadas = service->recalcular();
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"otorgadas", otorgadas}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void InsigniaController::otorgar(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    auto identity = requireAdmin(request, response);
    if (!identity) return;

    try {
        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const auto otorgamiento =
            OtorgamientoInsignia::fromJson(json::parse(request.body()));
        const bool nueva = service->otorgar(otorgamiento.usuarioId,
                                            otorgamiento.codigo,
                                            identity->userId,
                                            otorgamiento.motivo);

        // 200 y no 201 cuando ya la tenía: otorgar dos veces no es un error,
        // pero tampoco creó nada.
        sendJson(response,
                 nueva ? Pistache::Http::Code::Created : Pistache::Http::Code::Ok,
                 {{"success", true}, {"otorgada", nueva}});
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false},
                  {"error", "JSON inválido: " + std::string(error.what())}});
    } catch (const std::out_of_range& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void InsigniaController::revocar(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    auto identity = requireAdmin(request, response);
    if (!identity) return;

    try {
        const int usuarioId = request.param(":usuarioId").as<int>();
        const std::string codigo = request.param(":codigo").as<std::string>();

        if (!service->revocar(usuarioId, codigo)) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false},
                      {"error", "esa persona no tiene esa insignia"}});
            return;
        }

        sendJson(response, Pistache::Http::Code::Ok, {{"success", true}});
    } catch (const std::out_of_range& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}
