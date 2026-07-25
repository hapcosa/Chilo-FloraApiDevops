#ifndef REQUEST_IDENTITY_HPP
#define REQUEST_IDENTITY_HPP

#include <optional>
#include <string>

#include <pistache/http.h>
#include <pistache/router.h>

// Identidad confiable del usuario autenticado, propagada por el gateway
// (nginx auth_request -> X-User-Id / X-User-Role) tras verificar el JWT
// contra auth-service. Los controladores deben usar esto en vez de campos
// como creado_por/moderado_por del cuerpo de la petición: el cliente no
// puede autoatribuirse como otro usuario ni inventarse un rol.
struct RequestIdentity {
    int userId;
    std::string role;

    bool isAdmin() const { return role == "admin"; }
    bool isModerator() const { return role == "moderator"; }
    bool canModerate() const { return isAdmin() || isModerator(); }
};

// Devuelve la identidad si el gateway propagó X-User-Id/X-User-Role
// correctamente; std::nullopt si faltan o son inválidos (petición sin pasar
// por auth_request, o llamada directa a especies-api saltándose el gateway).
std::optional<RequestIdentity> extractIdentity(const Pistache::Rest::Request& request);

// Helpers de guardia reutilizables por cualquier controlador: si el chequeo
// falla, ya envían la respuesta de error (401/403) y devuelven std::nullopt
// para que el caller simplemente haga `if (!identity) return;`.
std::optional<RequestIdentity> requireAuthenticated(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter& response);
std::optional<RequestIdentity> requireModerador(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter& response);
std::optional<RequestIdentity> requireAdmin(
    const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter& response);

#endif  // REQUEST_IDENTITY_HPP
