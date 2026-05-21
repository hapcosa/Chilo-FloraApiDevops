#pragma once
#include <string>
#include <chrono>
#include <jwt-cpp/jwt.h>
#include <random>

class TokenUtils {
private:
    static std::string secretKey; // Clave secreta para firmar tokens JWT

public:
    // Configurar clave secreta (debería llamarse una vez al inicio de la aplicación)
    static void setSecretKey(const std::string& key) {
        secretKey = key;
    }

    // Generar un token JWT para un usuario
    static std::string generateJWT(int userId, const std::string& username, 
                                  const std::chrono::seconds& expirationTime = std::chrono::hours(24)) {
        auto now = std::chrono::system_clock::now();
        auto expiration = now + expirationTime;

        return jwt::create()
            .set_issuer("api.yourapp.com")
            .set_type("JWS")
            .set_issued_at(now)
            .set_expires_at(expiration)
            .set_subject(std::to_string(userId))
            .set_payload_claim("username", jwt::claim(username))
            .sign(jwt::algorithm::hs256{secretKey});
    }

    // Validar un token JWT
    static bool validateJWT(const std::string& token, int& outUserId) {
        try {
            auto decoded = jwt::decode(token);
            
            // Verificar la firma
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secretKey})
                .with_issuer("api.yourapp.com");
            
            verifier.verify(decoded);
            
            // Extraer ID de usuario del token
            outUserId = std::stoi(decoded.get_subject());
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }

    // Generar una clave segura para firmar tokens JWT
    static std::string generateRandomSecretKey(size_t length = 64) {
        const std::string chars = 
            "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()";
        
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, chars.size() - 1);
        
        std::string result;
        result.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            result += chars[distribution(generator)];
        }
        
        return result;
    }
};