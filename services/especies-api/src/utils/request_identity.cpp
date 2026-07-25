#include "../../include/utils/request_identity.hpp"

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
