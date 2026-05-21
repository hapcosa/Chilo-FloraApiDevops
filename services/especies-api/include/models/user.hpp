#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

class User {
private:
    int id;
    std::string username;
    std::string email;
    std::string password; // Almacenada en hash
    bool active;
    bool admin;
    std::string authToken;
    std::chrono::system_clock::time_point tokenExpiry;

public:
    // Constructor
    User(int id, const std::string& username, const std::string& email, 
         const std::string& password, bool active = true, bool admin = false)
        : id(id), username(username), email(email), password(password), 
          active(active), admin(admin) {}

    // Getters
    int getId() const { return id; }
    std::string getUsername() const { return username; }
    std::string getEmail() const { return email; }
    std::string getPassword() const { return password; }
    bool isActive() const { return active; }
    bool isAdmin() const { return admin; }
    std::string getAuthToken() const { return authToken; }
    std::chrono::system_clock::time_point getTokenExpiry() const { return tokenExpiry; }

    // Setters
    void setId(int newId) { id = newId; }
    void setUsername(const std::string& newUsername) { username = newUsername; }
    void setEmail(const std::string& newEmail) { email = newEmail; }
    void setPassword(const std::string& newPassword) { password = newPassword; }
    void setActive(bool newActive) { active = newActive; }
    void setAdmin(bool newAdmin) { admin = newAdmin; }
    void setAuthToken(const std::string& newToken) { authToken = newToken; }
    void setTokenExpiry(const std::chrono::system_clock::time_point& newExpiry) { tokenExpiry = newExpiry; }

    // Verificar si el token es válido (no expirado)
    bool hasValidToken() const {
        if (authToken.empty()) return false;
        auto now = std::chrono::system_clock::now();
        return now < tokenExpiry;
    }
    
    // Convertir a JSON (para APIs)
    nlohmann::json toJson() const {
        nlohmann::json json;
        json["id"] = id;
        json["username"] = username;
        json["email"] = email;
        json["active"] = active;
        json["admin"] = admin;
        // No incluimos password por seguridad
        return json;
    }
};
