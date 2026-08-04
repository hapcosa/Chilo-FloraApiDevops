#include "../../include/controllers/identificacion_controller.hpp"

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

IdentificacionController::IdentificacionController(
    std::shared_ptr<IdentificacionService> service)
    : service(std::move(service)) {}

std::optional<RequestIdentity> IdentificacionController::requireSesion(
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

void IdentificacionController::create(const Pistache::Rest::Request& request,
                                      Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        const int avistamientoId = request.param(":id").as<int>();

        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const auto payload = json::parse(request.body());
        const auto creada = service->identificar(avistamientoId,
                                                 identity->userId,
                                                 identity->role,
                                                 Identificacion::fromJson(payload));

        sendJson(response, Pistache::Http::Code::Created, creada.toJson());
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false},
                  {"error", "JSON inválido: " + std::string(error.what())}});
    } catch (const std::out_of_range& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::invalid_argument& error) {
        // El único conflicto posible es la identificación vigente duplicada:
        // el recurso ya existe, el cuerpo no tiene nada de malo.
        const std::string mensaje = error.what();
        const auto code =
            mensaje.rfind("ya identificaste", 0) == 0
                ? Pistache::Http::Code::Conflict
                : Pistache::Http::Code::Bad_Request;
        sendJson(response, code, {{"success", false}, {"error", mensaje}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void IdentificacionController::getAll(const Pistache::Rest::Request& request,
                                      Pistache::Http::ResponseWriter response) {
    try {
        const int avistamientoId = request.param(":id").as<int>();
        const auto identificaciones = service->getIdentificaciones(avistamientoId);

        json data = json::array();
        for (const auto& identificacion : identificaciones) {
            data.push_back(identificacion.toJson());
        }

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", data}});
    } catch (const std::out_of_range& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void IdentificacionController::retirar(const Pistache::Rest::Request& request,
                                       Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        const int avistamientoId = request.param(":id").as<int>();
        const int identificacionId = request.param(":idIdentificacion").as<int>();

        const auto existente = service->getIdentificacionById(identificacionId);
        // Colgar de otro avistamiento es un 404 y no un 400: desde fuera, esa
        // identificación no existe en esta ruta.
        if (!existente || existente->getAvistamientoId() != avistamientoId) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "identificación no encontrada"}});
            return;
        }

        if (!identity->isAdmin() && existente->getUsuarioId() != identity->userId) {
            sendJson(response, Pistache::Http::Code::Forbidden,
                     {{"success", false},
                      {"error", "Solo puedes retirar tus propias identificaciones"}});
            return;
        }

        const auto retirada = service->retirar(identificacionId);
        if (!retirada) {
            sendJson(response, Pistache::Http::Code::Conflict,
                     {{"success", false},
                      {"error", "la identificación ya estaba retirada"}});
            return;
        }

        sendJson(response, Pistache::Http::Code::Ok, retirada->toJson());
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}
