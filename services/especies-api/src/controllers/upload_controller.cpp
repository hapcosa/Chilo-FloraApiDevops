#include "../../include/controllers/upload_controller.hpp"

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

}  // namespace

UploadController::UploadController(
    std::shared_ptr<UploadPresignService> service)
    : service(std::move(service)) {}

void UploadController::presignPut(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
    try {
        if (request.body().empty()) {
            sendJson(response, Pistache::Http::Code::Bad_Request,
                     {{"success", false},
                      {"error", "El cuerpo de la petición no puede estar vacío"}});
            return;
        }

        const auto payload = json::parse(request.body());
        const std::string bucket = payload.value("bucket", "");
        const std::string filename = payload.value("filename", "");
        const std::string contentType = payload.value("content_type", "");

        std::optional<int> expiresIn;
        if (payload.contains("expires_in")) {
            if (!payload["expires_in"].is_number_integer()) {
                throw std::invalid_argument("'expires_in' debe ser entero");
            }
            expiresIn = payload["expires_in"].get<int>();
        }

        const PresignedUpload upload =
            service->createPresignedPut(bucket, filename, contentType, expiresIn);

        json headers = json::object();
        for (const auto& header : upload.headers) {
            headers[header.first] = header.second;
        }

        sendJson(response, Pistache::Http::Code::Ok,
                 {{"success", true},
                  {"method", upload.method},
                  {"bucket", upload.bucket},
                  {"key", upload.key},
                  {"url", upload.url},
                  {"headers", headers},
                  {"expires_in", upload.expiresIn}});
    } catch (const json::parse_error& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false},
                  {"error", "JSON inválido: " + std::string(error.what())}});
    } catch (const std::invalid_argument& error) {
        sendJson(response, Pistache::Http::Code::Bad_Request,
                 {{"success", false}, {"error", error.what()}});
    } catch (const std::exception& error) {
        sendJson(response, Pistache::Http::Code::Service_Unavailable,
                 {{"success", false}, {"error", error.what()}});
    }
}
