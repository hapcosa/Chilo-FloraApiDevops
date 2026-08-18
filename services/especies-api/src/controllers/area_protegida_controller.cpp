#include "../../include/controllers/area_protegida_controller.hpp"

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

std::optional<std::string> queryStr(const Pistache::Http::Uri::Query& query,
                                    const std::string& key) {
    if (!query.has(key)) return std::nullopt;
    return query.get(key).value();
}

// Mismo orden GeoJSON que el endpoint del mapa: min_lng,min_lat,max_lng,max_lat.
void aplicarBbox(AreaProtegidaFilters& filters, const std::string& bbox) {
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
}

} // namespace

AreaProtegidaController::AreaProtegidaController(
    std::shared_ptr<AreaProtegidaService> service)
    : service(std::move(service)) {}

void AreaProtegidaController::getAll(const Pistache::Rest::Request& request,
                                     Pistache::Http::ResponseWriter response) {
    try {
        const auto& query = request.query();
        AreaProtegidaFilters filters;

        if (const auto tipo = queryStr(query, "tipo")) {
            filters.tipo = areaProtegidaTipoFromString(*tipo);
        }
        if (const auto bbox = queryStr(query, "bbox")) {
            aplicarBbox(filters, *bbox);
        }

        const auto areas = service->listar(filters);

        json data = json::array();
        for (const auto& area : areas) {
            data.push_back(area.toJson());
        }
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", data}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception&) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", "Error al listar áreas protegidas"}});
    }
}

void AreaProtegidaController::getById(const Pistache::Rest::Request& request,
                                      Pistache::Http::ResponseWriter response) {
    try {
        const int id = request.param(":id").as<int>();
        const auto area = service->obtener(id);
        if (!area) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "Área protegida no encontrada"}});
            return;
        }
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", area->toJson()}});
    } catch (const std::exception&) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", "Identificador inválido"}});
    }
}

void AreaProtegidaController::getEspecies(const Pistache::Rest::Request& request,
                                          Pistache::Http::ResponseWriter response) {
    try {
        const int id = request.param(":id").as<int>();
        // 404 antes de listar: "el parque no existe" y "el parque no tiene
        // registros" son respuestas distintas para quien está planeando un viaje.
        if (!service->obtener(id)) {
            sendJson(response, Pistache::Http::Code::Not_Found,
                     {{"success", false}, {"error", "Área protegida no encontrada"}});
            return;
        }

        int limit = 0;
        if (const auto valor = queryStr(request.query(), "limit")) {
            try {
                limit = std::stoi(*valor);
            } catch (...) {
                throw std::invalid_argument("'limit' debe ser entero");
            }
        }

        const auto especies = service->especiesEnArea(id, limit);
        json data = json::array();
        for (const auto& especie : especies) {
            data.push_back(especie.toJson());
        }
        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true}, {"data", data}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception&) {
        sendJson(response, Pistache::Http::Code::Internal_Server_Error,
                 {{"success", false}, {"error", "Error al listar especies del área"}});
    }
}
