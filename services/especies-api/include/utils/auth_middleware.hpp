#pragma once

#include <functional>
#include <pistache/http.h>
#include <pistache/router.h>
#include <memory>
#include "../services/user_service.hpp"
#include "../models/user.hpp"
#include <nlohmann/json.hpp>

/**
 * @class AuthMiddleware
 * @brief Middleware para autenticación JWT en rutas protegidas
 */
class AuthMiddleware {
private:
    std::shared_ptr<UserService> userService;

public:
    /**
     * @brief Constructor que recibe el servicio de usuario
     * @param userService Puntero compartido al UserService
     */
    explicit AuthMiddleware(std::shared_ptr<UserService> userService) 
        : userService(std::move(userService)) {}

    /**
     * @brief Decorador para rutas que requieren autenticación
     * @tparam Handler Tipo del manejador de ruta
     * @param handler Manejador original de la ruta
     * @return Handler protegido por autenticación
     */
    template<typename Handler>
    Pistache::Rest::Route::Handler authRequired(Handler&& handler) {
        return [this, handler = std::forward<Handler>(handler)](
            const Pistache::Rest::Request& request, 
            Pistache::Http::ResponseWriter response) {
            
            // 1. Extraer token del header
            if (!validateAuthHeader(request, response)) {
                return;
            }

            // 2. Verificar token
            auto userOpt = validateToken(request, response);
            if (!userOpt) {
                return;
            }

            // 3. Si la autenticación es exitosa, pasar al handler
            handler(request, std::move(response), userOpt.value());
        };
    }

private:
    /**
     * @brief Valida el encabezado de autorización
     * @return true si el header es válido, false en caso contrario
     */
    bool validateAuthHeader(
        const Pistache::Rest::Request& request, 
        Pistache::Http::ResponseWriter& response) const;
    
    /**
     * @brief Valida el token JWT
     * @return std::optional<User> con el usuario si el token es válido
     */
    std::optional<User> validateToken(
        const Pistache::Rest::Request& request,
        Pistache::Http::ResponseWriter& response) const;
    
    /**
     * @brief Envía una respuesta de error de autenticación
     */
    void sendAuthError(
        Pistache::Http::ResponseWriter& response, 
        const std::string& message, 
        Pistache::Http::Code code) const;
};