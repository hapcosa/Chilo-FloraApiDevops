#include "../../include/controllers/schema_controller.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <stdexcept>
#include <utility>

using json = nlohmann::json;

namespace {

constexpr std::array<Reino, 5> kReinos = {Reino::Animalia, Reino::Plantae,
                                          Reino::Fungi, Reino::Protista,
                                          Reino::Monera};

void sendJson(Pistache::Http::ResponseWriter& response,
              Pistache::Http::Code code,
              const json& payload) {
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(code, payload.dump());
}

}  // namespace

SchemaController::SchemaController(
    std::shared_ptr<AtributosSchemaValidator> validator)
    : validator(std::move(validator)) {}

void SchemaController::getAll(const Pistache::Rest::Request&,
                              Pistache::Http::ResponseWriter response) {
    json data = json::object();
    for (const auto reino : kReinos) {
        data[reinoToString(reino)] = validator->schemaDe(reino);
    }
    sendJson(response, Pistache::Http::Code::Ok,
             {{"success", true}, {"data", data}});
}

void SchemaController::getByReino(const Pistache::Rest::Request& request,
                                  Pistache::Http::ResponseWriter response) {
    const auto reinoParam = request.param(":reino").as<std::string>();
    if (!isValidReino(reinoParam)) {
        sendJson(response, Pistache::Http::Code::Not_Found,
                 {{"success", false},
                  {"error", "Reino desconocido: " + reinoParam}});
        return;
    }

    sendJson(response, Pistache::Http::Code::Ok,
             validator->schemaDe(reinoFromString(reinoParam)));
}
