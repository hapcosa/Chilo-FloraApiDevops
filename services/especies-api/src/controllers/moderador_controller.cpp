#include "../../include/controllers/moderador_controller.hpp"

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

ModeradorController::ModeradorController(std::shared_ptr<ModeracionService> service)
    : service(std::move(service)) {}

std::optional<RequestIdentity> ModeradorController::requireAdmin(
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

void ModeradorController::asignar(const Pistache::Rest::Request& request,
                                  Pistache::Http::ResponseWriter response) {
    auto identity = requireAdmin(request, response);
    if (!identity) return;

    try {
        const int categoriaId = request.param(":id").as<int>();
        const int usuarioId = request.param(":usuarioId").as<int>();

        const bool creada =
            service->asignarCurador(usuarioId, categoriaId, identity->userId);

        sendJson(response,
                 creada ? Pistache::Http::Code::Created : Pistache::Http::Code::Ok,
                 {{"success", true},
                  {"usuario_id", usuarioId},
                  {"categoria_id", categoriaId},
                  {"message", creada ? "curador asignado"
                                     : "el curador ya tenía esta categoría"}});
    } catch (const std::out_of_range& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void ModeradorController::quitar(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
    if (!requireAdmin(request, response)) return;

    try {
        const int categoriaId = request.param(":id").as<int>();
        const int usuarioId = request.param(":usuarioId").as<int>();

        if (!service->quitarCurador(usuarioId, categoriaId)) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false},
                      {"error", "el usuario no cura esta categoría"}});
            return;
        }

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"message", "curaduría retirada"}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void ModeradorController::categoriasDeUsuario(const Pistache::Rest::Request& request,
                                              Pistache::Http::ResponseWriter response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false},
                  {"error", "No se pudo verificar la sesión del usuario"}});
        return;
    }

    try {
        const int usuarioId = request.param(":usuarioId").as<int>();

        // Un curador necesita saber qué cura para que el panel le muestre solo
        // lo suyo; el resto de usuarios no es asunto suyo.
        if (!identity->isAdmin() && identity->userId != usuarioId) {
            sendJson(response, Pistache::Http::Code::Forbidden,
                     {{"success", false},
                      {"error", "Solo puedes consultar tus propias categorías"}});
            return;
        }

        json data = json::array();
        for (const auto& categoria : service->categoriasDe(usuarioId)) {
            data.push_back(categoria.toJson());
        }

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", data}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}
