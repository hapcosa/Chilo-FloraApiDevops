#include "../../include/controllers/postulacion_controller.hpp"

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

PostulacionController::PostulacionController(std::shared_ptr<PostulacionService> service)
    : service(std::move(service)) {}

std::optional<RequestIdentity> PostulacionController::requireSesion(
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

std::optional<RequestIdentity> PostulacionController::requireAdmin(
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

void PostulacionController::create(const Pistache::Rest::Request& request,
                                   Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const auto payload = json::parse(request.body());
        const auto creada = service->postular(
            identity->userId, PostulacionCurador::fromJson(payload));

        sendJson(response, Pistache::Http::Code::Created, creada.toJson());
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

void PostulacionController::getAll(const Pistache::Rest::Request& request,
                                   Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        std::vector<PostulacionCurador> postulaciones;

        if (identity->isAdmin()) {
            std::optional<PostulacionEstado> estado;
            const auto query = request.query();
            if (query.has("estado")) {
                estado = postulacionEstadoFromString(query.get("estado").value());
            }
            postulaciones = service->getPostulaciones(estado);
        } else {
            // Sin bandeja ajena: quién más se postuló no es asunto suyo. El
            // filtro por estado no aplica porque son pocas y propias.
            postulaciones = service->getPostulacionesDe(identity->userId);
        }

        json data = json::array();
        for (const auto& postulacion : postulaciones) {
            data.push_back(postulacion.toJson());
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

void PostulacionController::getById(const Pistache::Rest::Request& request,
                                    Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        const int id = request.param(":id").as<int>();
        const auto postulacion = service->getPostulacionById(id);
        if (!postulacion) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "postulación no encontrada"}});
            return;
        }

        if (!identity->isAdmin() && postulacion->getUsuarioId() != identity->userId) {
            sendJson(response, Pistache::Http::Code::Forbidden,
                     {{"success", false},
                      {"error", "Solo puedes consultar tus propias postulaciones"}});
            return;
        }

        sendJson(response, Pistache::Http::Code::Ok, postulacion->toJson());
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void PostulacionController::resolver(const Pistache::Rest::Request& request,
                                     Pistache::Http::ResponseWriter response) {
    auto identity = requireAdmin(request, response);
    if (!identity) return;

    try {
        const int id = request.param(":id").as<int>();

        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const auto payload = json::parse(request.body());
        if (!payload.contains("estado") || !payload["estado"].is_string()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "'estado' es obligatorio"}});
            return;
        }

        const auto estado =
            postulacionEstadoFromString(payload["estado"].get<std::string>());

        // Un 404 antes de intentar resolver: sin esto, una postulación
        // inexistente se confundiría con una ya resuelta (ambas 0 filas).
        if (!service->getPostulacionById(id)) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "postulación no encontrada"}});
            return;
        }

        PostulacionCurador resuelta;
        switch (estado) {
            case PostulacionEstado::Aprobada:
                resuelta = service->aprobar(id, identity->userId);
                break;
            case PostulacionEstado::Rechazada: {
                std::string motivo;
                if (payload.contains("motivo") && payload["motivo"].is_string()) {
                    motivo = payload["motivo"].get<std::string>();
                }
                resuelta = service->rechazar(id, identity->userId, motivo);
                break;
            }
            case PostulacionEstado::Pendiente:
                sendJson(response, Pistache::Http::Code::Bad_Request,
                         {{"success", false},
                          {"error", "'estado' debe ser 'aprobada' o 'rechazada'"}});
                return;
        }

        sendJson(response, Pistache::Http::Code::Ok, resuelta.toJson());
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false},
                  {"error", "JSON inválido: " + std::string(error.what())}});
    } catch (const std::invalid_argument& error) {
        // Incluye "la postulación ya fue resuelta": es un conflicto de estado,
        // no un cuerpo mal formado.
        const std::string mensaje = error.what();
        const auto code = mensaje == "la postulación ya fue resuelta"
                              ? Pistache::Http::Code::Conflict
                              : Pistache::Http::Code::Bad_Request;
        sendJson(response, code, {{"success", false}, {"error", mensaje}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}
