#pragma once
#include <string>
#include <bcrypt/BCrypt.hpp>

class PasswordUtils {
public:
    // Generar hash de la contraseña usando bcrypt
    static std::string hashPassword(const std::string& password) {
        return BCrypt::generateHash(password);
    }
    
    // Verificar contraseña contra un hash
    static bool verifyPassword(const std::string& password, const std::string& hash) {
        return BCrypt::validatePassword(password, hash);
    }
};