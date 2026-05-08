#include "../../include/utils/auth_middleware.hpp"
#include <nlohmann/json.hpp>

using namespace Pistache;

bool AuthMiddleware::validateAuthHeader(
    const Rest::Request& request, 
    Http::ResponseWriter& response) const 
{
    auto authHeader = request.headers().tryGet<Http::Header::Authorization>();
    if (!authHeader) {
        sendAuthError(response, "Authorization header required", Http::Code::Unauthorized);
        return false;
    }

    auto auth = authHeader->value();
    if (auth.find("Bearer ") != 0) {
        sendAuthError(response, 
            "Invalid authorization format. Use 'Bearer [token]'", 
            Http::Code::Unauthorized);
        return false;
    }

    return true;
}

std::optional<User> AuthMiddleware::validateToken(
    const Rest::Request& request,
    Http::ResponseWriter& response) const 
{
    auto authHeader = request.headers().get<Http::Header::Authorization>();
    std::string token = authHeader->value().substr(7); // Remove "Bearer "
    
    auto userOpt = userService->validateToken(token);
    if (!userOpt) {
        sendAuthError(response, "Invalid or expired token", Http::Code::Unauthorized);
    }
    
    return userOpt;
}

void AuthMiddleware::sendAuthError(
    Http::ResponseWriter& response, 
    const std::string& message, 
    Http::Code code) const 
{
    nlohmann::json error = {{"message", message}};
    response.headers().add<Http::Header::ContentType>(
        Http::Mime::MediaType("application", "json"));
    response.send(code, error.dump());
}