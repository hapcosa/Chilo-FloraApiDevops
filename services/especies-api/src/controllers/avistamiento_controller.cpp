#include "../../include/controllers/avistamiento_controller.hpp"
#include "../../include/services/avistamiento_visibilidad.hpp"
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

VisibilidadSolicitante solicitanteDe(const Pistache::Rest::Request& request) {
    VisibilidadSolicitante solicitante;
    if (const auto identity = extractIdentity(request)) {
        solicitante.usuario_id = identity->userId;
        solicitante.puede_moderar = identity->canModerate();
    }
    return solicitante;
}

// "min_lng,min_lat,max_lng,max_lat" — el orden de GeoJSON y de las APIs de
// mapas, para no obligar al cliente a reordenar lo que ya tiene.
MapaFilters parseBbox(const std::string& bbox) {
    MapaFilters filters;
    double valores[4] = {0, 0, 0, 0};
    std::size_t inicio = 0;
    for (int i = 0; i < 4; ++i) {
        const auto coma = bbox.find(',', inicio);
        if (i < 3 && coma == std::string::npos) {
            throw std::invalid_argument("'bbox' debe ser min_lng,min_lat,max_lng,max_lat");
        }
        const auto trozo = bbox.substr(inicio, coma == std::string::npos ? coma : coma - inicio);
        try {
            valores[i] = std::stod(trozo);
        } catch (...) {
            throw std::invalid_argument("'bbox' debe ser min_lng,min_lat,max_lng,max_lat");
        }
        inicio = coma == std::string::npos ? bbox.size() : coma + 1;
    }
    filters.min_lng = valores[0];
    filters.min_lat = valores[1];
    filters.max_lng = valores[2];
    filters.max_lat = valores[3];
    return filters;
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

// Celdas agregadas para dibujar el mapa. No devuelve avistamientos: ni el
// cliente necesita diez mil puntos para pintar un clúster, ni conviene publicar
// diez mil ubicaciones exactas de una sola petición.
void AvistamientoController::getMapa(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    try {
        const auto query = request.query();
        const auto bbox = queryStr(query, "bbox");
        if (!bbox) {
            throw std::invalid_argument("'bbox' es obligatorio");
        }

        auto filters = parseBbox(*bbox);
        if (auto value = queryInt(query, "zoom")) {
            filters.zoom = *value;
        }
        if (auto value = queryStr(query, "reino")) {
            filters.reino = reinoFromString(*value);
        }
        if (auto value = queryInt(query, "especie_id")) {
            filters.especie_id = *value;
        }

        json data = json::array();
        for (const auto& celda : service->mapaAvistamientos(filters)) {
            data.push_back(celda.toJson());
        }

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", data}, {"zoom", filters.zoom}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", error.what()}});
    }
}

void AvistamientoController::getAll(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    try {
        const auto query = request.query();
        AvistamientoFilters filters;
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
        if (auto value = queryStr(query, "visibilidad")) {
            filters.visibilidad = avistamientoVisibilidadFromString(*value);
        }
        if (auto value = queryStr(query, "grado_identificacion")) {
            filters.grado_identificacion = gradoIdentificacionFromString(*value);
        }
        if (auto value = queryInt(query, "limit")) {
            filters.limit = *value;
        }
        if (auto value = queryInt(query, "offset")) {
            filters.offset = *value;
        }

        // El estado pedido solo se respeta si quien pregunta tiene derecho a
        // verlo; el resto de las peticiones se acotan a 'aprobado'.
        filters = restringirVisibilidad(filters, solicitanteDe(request));

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
    try {
        const int id = request.param(":id").as<int>();
        const auto avistamiento = service->getAvistamientoById(id);
        // La misma regla que el listado: si no está aprobado solo lo ven su
        // autor y quien modera. 404 y no 403 para no confirmar que existe.
        if (!avistamiento || !puedeVerAvistamiento(*avistamiento, solicitanteDe(request))) {
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
        // Solo el autor publica lo suyo. 404 y no 403 si es de otro: confirmar
        // que existe ya diría algo de un encuentro privado ajeno.
        const auto avistamiento = service->compartirAvistamiento(id, identity->userId);
        if (!avistamiento) {
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

