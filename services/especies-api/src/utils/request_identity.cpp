#include "../../include/utils/request_identity.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::optional<RequestIdentity> extractIdentity(const Pistache::Rest::Request& request) {
  auto idHeader = request.headers().tryGetRaw("X-User-Id");
  auto roleHeader = request.headers().tryGetRaw("X-User-Role");
  if (!idHeader || !roleHeader || idHeader->value().empty() || roleHeader->value().empty()) {
    return std::nullopt;
  }

  try {
    int userId = std::stoi(idHeader->value());
    return RequestIdentity{userId, roleHeader->value()};
  } catch (...) {
    return std::nullopt;
  }
}

namespace {

void sendError(Pistache::Http::ResponseWriter& response, Pistache::Http::Code code,
               const std::string& message) {
  json error = {{"error", message}};
  response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
  response.send(code, error.dump());
}

}  // namespace

std::optional<RequestIdentity> requireAuthenticated(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter& response) {
  auto identity = extractIdentity(request);
  if (!identity) {
    sendError(response, Pistache::Http::Code::Unauthorized,
              "No se pudo verificar la sesión del usuario");
    return std::nullopt;
  }
  return identity;
}

std::optional<RequestIdentity> requireModerador(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter& response) {
  auto identity = requireAuthenticated(request, response);
  if (!identity) return std::nullopt;
  if (!identity->canModerate()) {
    sendError(response, Pistache::Http::Code::Forbidden, "Se requiere rol admin o moderator");
    return std::nullopt;
  }
  return identity;
}

std::optional<RequestIdentity> requireAdmin(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter& response) {
  auto identity = requireAuthenticated(request, response);
  if (!identity) return std::nullopt;
  if (!identity->isAdmin()) {
    sendError(response, Pistache::Http::Code::Forbidden, "Se requiere rol admin");
    return std::nullopt;
  }
  return identity;
}
