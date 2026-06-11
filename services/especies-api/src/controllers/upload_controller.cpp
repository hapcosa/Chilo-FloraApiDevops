#include "../../include/controllers/upload_controller.hpp"

#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

UploadController::UploadController(std::shared_ptr<UploadService> service)
    : service(std::move(service)) {}

void UploadController::presign(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
    try {
        if (request.body().empty()) {
            json error = {{"success", false},
                          {"error", "El cuerpo de la petición no puede estar vacío"}};
            response.headers().add<Pistache::Http::Header::ContentType>(
                MIME(Application, Json));
            response.send(Pistache::Http::Code::Bad_Request, error.dump());
            return;
        }

        const auto body = json::parse(request.body());
        if (!body.contains("content_type") || !body["content_type"].is_string()) {
            json error = {{"success", false},
                          {"error", "content_type es obligatorio"}};
            response.headers().add<Pistache::Http::Header::ContentType>(
                MIME(Application, Json));
            response.send(Pistache::Http::Code::Bad_Request, error.dump());
            return;
        }

        PresignUploadRequest presignRequest;
        presignRequest.scope = body.value("scope", "especies");
        presignRequest.contentType = body["content_type"].get<std::string>();
        presignRequest.filename = body.value("filename", "");
        if (body.contains("expires_seconds") && !body["expires_seconds"].is_null()) {
            if (!body["expires_seconds"].is_number_integer()) {
                throw std::invalid_argument("expires_seconds debe ser entero");
            }
            presignRequest.expiresSeconds = body["expires_seconds"].get<int>();
        }

        const auto presigned = service->createPresignedUpload(presignRequest);
        json out = {{"success", true}, {"data", presigned.toJson()}};

        response.headers().add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json));
        response.send(Pistache::Http::Code::Created, out.dump());
    } catch (const json::parse_error& e) {
        json error = {{"success", false},
                      {"error", "JSON inválido: " + std::string(e.what())}};
        response.headers().add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json));
        response.send(Pistache::Http::Code::Bad_Request, error.dump());
    } catch (const std::invalid_argument& e) {
        json error = {{"success", false}, {"error", e.what()}};
        response.headers().add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json));
        response.send(Pistache::Http::Code::Bad_Request, error.dump());
    } catch (const std::exception& e) {
        json error = {{"success", false}, {"error", e.what()}};
        response.headers().add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json));
        response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
    }
}
