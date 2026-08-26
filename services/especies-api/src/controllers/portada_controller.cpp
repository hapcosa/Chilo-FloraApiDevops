#include "../../include/controllers/portada_controller.hpp"
#include "../../include/utils/query_params.hpp"

#include <nlohmann/json.hpp>

#include <optional>
#include <stdexcept>
#include <string>
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

// Decodifica el valor: Pistache lo entrega tal como viajó por la red.
std::optional<std::string> queryStr(const Pistache::Http::Uri::Query& query,
                                    const std::string& key) {
    if (!query.has(key)) return std::nullopt;
    return utils::percentDecode(query.get(key).value());
}

} // namespace

PortadaController::PortadaController(std::shared_ptr<PortadaService> service)
    : service(std::move(service)) {}

void PortadaController::getPortada(const Pistache::Rest::Request& request,
                                   Pistache::Http::ResponseWriter response) {
    try {
        int limite = PortadaService::kLimitePorDefecto;
        if (const auto valor = queryStr(request.query(), "limite")) {
            try {
                limite = std::stoi(*valor);
            } catch (...) {
                throw std::invalid_argument("'limite' debe ser entero");
            }
        }

        // El servicio acota el rango; aquí no se valida el máximo para no
        // tener el mismo número escrito en dos sitios.
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", service->obtenerPortada(limite).toJson()}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception&) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", "Error al construir la portada"}});
    }
}
