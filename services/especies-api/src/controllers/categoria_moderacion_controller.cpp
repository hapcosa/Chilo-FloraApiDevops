#include "../../include/controllers/categoria_moderacion_controller.hpp"
#include "../../include/utils/request_identity.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

using json = nlohmann::json;

namespace {

void sendJson(Pistache::Http::ResponseWriter& response, Pistache::Http::Code code,
              const json& payload) {
  response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
  response.send(code, payload.dump());
}

std::optional<int> paramInt(const Pistache::Rest::Request& request, const std::string& name) {
  try {
    return request.param(":" + name).as<int>();
  } catch (...) {
    return std::nullopt;
  }
}

}  // namespace

CategoriaModeracionController::CategoriaModeracionController(
    std::shared_ptr<CategoriaModeracionService> svc)
    : service(std::move(svc)) {}

void CategoriaModeracionController::getAll(const Pistache::Rest::Request& request,
                                           Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    json data = json::array();
    for (const auto& categoria : service->getAll()) {
      data.push_back(categoria.toJson());
    }
    sendJson(response, Pistache::Http::Code::Ok, data);
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}

void CategoriaModeracionController::getById(const Pistache::Rest::Request& request,
                                            Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    const int id = request.param(":id").as<int>();
    const auto categoria = service->findById(id);
    if (!categoria) {
      sendJson(response, Pistache::Http::Code::Not_Found,
               {{"error", "Categoría de moderación no encontrada"}});
      return;
    }
    sendJson(response, Pistache::Http::Code::Ok, categoria->toJson());
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}

void CategoriaModeracionController::create(const Pistache::Rest::Request& request,
                                           Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    if (request.body().empty()) {
      sendJson(response, Pistache::Http::Code::Bad_Request,
               {{"error", "El cuerpo de la petición no puede estar vacío"}});
      return;
    }
    const auto payload = json::parse(request.body());
    const auto categoria = service->create(CategoriaModeracion::fromJson(payload));
    sendJson(response, Pistache::Http::Code::Created, categoria.toJson());
  } catch (const json::parse_error& e) {
    sendJson(response, Pistache::Http::Code::Bad_Request,
             {{"error", "JSON inválido: " + std::string(e.what())}});
  } catch (const std::invalid_argument& e) {
    sendJson(response, Pistache::Http::Code::Bad_Request, {{"error", e.what()}});
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}

void CategoriaModeracionController::update(const Pistache::Rest::Request& request,
                                           Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    const int id = request.param(":id").as<int>();
    if (request.body().empty()) {
      sendJson(response, Pistache::Http::Code::Bad_Request,
               {{"error", "El cuerpo de la petición no puede estar vacío"}});
      return;
    }
    const auto payload = json::parse(request.body());
    auto categoria = CategoriaModeracion::fromJson(payload);
    categoria.setId(id);
    const auto actualizada = service->update(categoria);
    sendJson(response, Pistache::Http::Code::Ok, actualizada.toJson());
  } catch (const json::parse_error& e) {
    sendJson(response, Pistache::Http::Code::Bad_Request,
             {{"error", "JSON inválido: " + std::string(e.what())}});
  } catch (const std::invalid_argument& e) {
    sendJson(response, Pistache::Http::Code::Bad_Request, {{"error", e.what()}});
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}

void CategoriaModeracionController::remove(const Pistache::Rest::Request& request,
                                           Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    const int id = request.param(":id").as<int>();
    service->remove(id);
    sendJson(response, Pistache::Http::Code::No_Content, {});
  } catch (const std::invalid_argument& e) {
    sendJson(response, Pistache::Http::Code::Not_Found, {{"error", e.what()}});
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}

void CategoriaModeracionController::listModeradores(const Pistache::Rest::Request& request,
                                                     Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    const int id = request.param(":id").as<int>();
    json data = json::array();
    for (int userId : service->listModeradores(id)) {
      data.push_back(userId);
    }
    sendJson(response, Pistache::Http::Code::Ok, {{"categoria_moderacion_id", id}, {"moderadores", data}});
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}

void CategoriaModeracionController::assignModerador(const Pistache::Rest::Request& request,
                                                     Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    const int id = request.param(":id").as<int>();
    const auto userId = paramInt(request, "user_id");
    if (!userId) {
      sendJson(response, Pistache::Http::Code::Bad_Request, {{"error", "user_id inválido"}});
      return;
    }
    service->assignModerador(id, *userId);
    sendJson(response, Pistache::Http::Code::Created,
             {{"categoria_moderacion_id", id}, {"user_id", *userId}});
  } catch (const std::invalid_argument& e) {
    sendJson(response, Pistache::Http::Code::Not_Found, {{"error", e.what()}});
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}

void CategoriaModeracionController::unassignModerador(const Pistache::Rest::Request& request,
                                                       Pistache::Http::ResponseWriter response) {
  if (!requireAdmin(request, response)) return;

  try {
    const int id = request.param(":id").as<int>();
    const auto userId = paramInt(request, "user_id");
    if (!userId) {
      sendJson(response, Pistache::Http::Code::Bad_Request, {{"error", "user_id inválido"}});
      return;
    }
    service->unassignModerador(id, *userId);
    sendJson(response, Pistache::Http::Code::No_Content, {});
  } catch (const std::exception& e) {
    sendJson(response, Pistache::Http::Code::Internal_Server_Error, {{"error", e.what()}});
  }
}
