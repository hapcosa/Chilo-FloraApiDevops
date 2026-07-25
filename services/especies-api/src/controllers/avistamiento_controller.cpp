#include "../../include/controllers/avistamiento_controller.hpp"
#include "../../include/utils/request_identity.hpp"

#include <nlohmann/json.hpp>

#include <optional>
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

std::optional<int> queryInt(const Pistache::Http::Uri::Query& query,
                            const std::string& key) {
    if (!query.has(key)) return std::nullopt;
    try {
        return std::stoi(query.get(key).value());
    } catch (...) {
        throw std::invalid_argument("Parámetro '" + key + "' debe ser entero");
    }
}

std::optional<std::string> queryStr(const Pistache::Http::Uri::Query& query,
                                    const std::string& key) {
    if (!query.has(key)) return std::nullopt;
    return query.get(key).value();
}

ModeracionAvistamiento parseModeracion(const json& payload) {
    if (!payload.contains("estado") || !payload["estado"].is_string()) {
        throw std::invalid_argument("'estado' es obligatorio");
    }

    ModeracionAvistamiento moderacion;
    moderacion.estado = avistamientoEstadoFromString(payload["estado"].get<std::string>());
    // moderado_por NO se toma del body: lo asigna el caller con la identidad
    // verificada del gateway (ver AvistamientoController::moderate).

    if (payload.contains("motivo_rechazo") && !payload["motivo_rechazo"].is_null()) {
        if (!payload["motivo_rechazo"].is_string()) {
            throw std::invalid_argument("'motivo_rechazo' debe ser string");
        }
        moderacion.motivo_rechazo = payload["motivo_rechazo"].get<std::string>();
    }

    return moderacion;
}

} // namespace

AvistamientoController::AvistamientoController(
    std::shared_ptr<AvistamientoService> service)
    : service(std::move(service)) {}

void AvistamientoController::getAll(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false}, {"error", "No se pudo verificar la sesión del usuario"}});
        return;
    }

    try {
        const auto query = request.query();
        AvistamientoFilters filters;
        filters.viewerUserId = identity->userId;
        filters.viewerIsAdmin = identity->isAdmin();
        if (auto value = queryStr(query, "estado")) {
            filters.estado = avistamientoEstadoFromString(*value);
        }
        if (auto value = queryStr(query, "reino")) {
            filters.reino = reinoFromString(*value);
        }
        if (auto value = queryInt(query, "especie_id")) {
            filters.especie_id = *value;
        }
        if (auto value = queryInt(query, "creado_por")) {
            filters.creado_por = *value;
        }
        if (auto value = queryInt(query, "limit")) {
            filters.limit = *value;
        }
        if (auto value = queryInt(query, "offset")) {
            filters.offset = *value;
        }

        const auto result = service->searchAvistamientos(filters);
        json data = json::array();
        for (const auto& avistamiento : result.data) {
            data.push_back(avistamiento.toJson());
        }

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true},
                  {"data", data},
                  {"pagination", {{"limit", filters.limit},
                                  {"offset", filters.offset},
                                  {"total", result.total}}}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void AvistamientoController::getById(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false}, {"error", "No se pudo verificar la sesión del usuario"}});
        return;
    }

    try {
        const int id = request.param(":id").as<int>();
        const auto avistamiento = service->getAvistamientoById(id);
        if (!avistamiento) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "avistamiento no encontrado"}});
            return;
        }

        const bool esDueno = avistamiento->getCreadoPor() &&
                             *avistamiento->getCreadoPor() == identity->userId;
        const bool esPublico = avistamiento->getVisibilidad() == AvistamientoVisibilidad::Publico;
        if (!esPublico && !esDueno && !identity->isAdmin()) {
            // 404 en vez de 403: no revelar que existe un avistamiento
            // privado de otro usuario con ese ID.
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "avistamiento no encontrado"}});
            return;
        }

        sendJson(response, Pistache::Http::Code::Ok, avistamiento->toJson());
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void AvistamientoController::create(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false}, {"error", "No se pudo verificar la sesión del usuario"}});
        return;
    }

    try {
        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const auto payload = json::parse(request.body());
        auto nuevoAvistamiento = Avistamiento::fromJson(payload);
        // creado_por viene de la identidad verificada, no del body del cliente.
        nuevoAvistamiento.setCreadoPor(identity->userId);
        const auto avistamiento = service->createAvistamiento(nuevoAvistamiento);

        sendJson(response, Pistache::Http::Code::Created, avistamiento.toJson());
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", "JSON inválido: " + std::string(error.what())}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void AvistamientoController::moderate(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false}, {"error", "No se pudo verificar la sesión del usuario"}});
        return;
    }
    if (!identity->canModerate()) {
        sendJson(response, Pistache::Http::Code::Forbidden,
                 {{"success", false}, {"error", "Se requiere rol admin o moderator"}});
        return;
    }

    try {
        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "El cuerpo no puede estar vacío"}});
            return;
        }

        const int id = request.param(":id").as<int>();
        const auto payload = json::parse(request.body());
        auto moderacion = parseModeracion(payload);
        // moderado_por viene de la identidad verificada, no del body del cliente.
        moderacion.moderado_por = identity->userId;
        const auto updated = service->moderateAvistamiento(id, moderacion);

        sendJson(response, Pistache::Http::Code::Ok, updated.toJson());
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", "JSON inválido: " + std::string(error.what())}});
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

void AvistamientoController::compartir(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    auto identity = extractIdentity(request);
    if (!identity) {
        sendJson(response, Pistache::Http::Code::Unauthorized,
                 {{"success", false}, {"error", "No se pudo verificar la sesión del usuario"}});
        return;
    }

    try {
        const int id = request.param(":id").as<int>();
        const auto updated = service->compartirAvistamiento(id, identity->userId);
        sendJson(response, Pistache::Http::Code::Ok, updated.toJson());
    } catch (const std::domain_error& error) {
        sendJson(response, Pistache::Http::Code::Forbidden,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

