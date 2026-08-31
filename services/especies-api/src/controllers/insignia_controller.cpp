#include "../../include/controllers/insignia_controller.hpp"
#include "../../include/utils/query_params.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <optional>
#include <sstream>
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

// Decodifica el valor: Pistache lo entrega tal como viajó por la red.
std::optional<std::string> queryStr(const Pistache::Http::Uri::Query& query,
                                    const std::string& key) {
    if (!query.has(key)) return std::nullopt;
    return utils::percentDecode(query.get(key).value());
}

// `1,2,3` → {1, 2, 3}. Un id que no sea entero es un error del cliente, no una
// lista más corta: callarlo devolvería insignias de gente que no se pidió.
std::vector<int> parseIds(const std::string& valor) {
    std::vector<int> ids;
    std::stringstream flujo(valor);
    std::string parte;
    while (std::getline(flujo, parte, ',')) {
        if (parte.empty()) continue;
        try {
            std::size_t consumidos = 0;
            const int id = std::stoi(parte, &consumidos);
            if (consumidos != parte.size()) throw std::invalid_argument("sobra texto");
            ids.push_back(id);
        } catch (const std::exception&) {
            throw std::invalid_argument("'ids' debe ser una lista de enteros separados por coma");
        }
    }
    return ids;
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

void InsigniaController::getDeUsuarios(const Pistache::Rest::Request& request,
                                       Pistache::Http::ResponseWriter response) {
    auto identity = requireSesion(request, response);
    if (!identity) return;

    try {
        const auto valor = queryStr(request.query(), "ids");
        if (!valor) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false}, {"error", "Falta el parámetro 'ids'"}});
            return;
        }

        // Objeto y no arreglo: el cliente busca por id, no recorre.
        json data = json::object();
        for (const auto& [usuarioId, otorgadas] :
             service->getInsigniasDeVarios(parseIds(*valor))) {
            data[std::to_string(usuarioId)] = toArray(otorgadas);
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
