#include "../models/user.hpp"
#include <iomanip>
#include <sstream>

User::User(int id, const std::string& username, const std::string& email, 
           const std::string& password, bool active) 
    : id(id), username(username), email(email), password(password), active(active) {
    // Set current timestamp
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
    createdAt = ss.str();
}

bool User::hasValidToken() const {
    if (!authToken.has_value() || !tokenExpiry.has_value()) {
        return false;
    }
    
    auto now = std::chrono::system_clock::now();
    return now < tokenExpiry.value();
}

nlohmann::json User::toJson() const {
    nlohmann::json j;
    j["id"] = id;
    j["username"] = username;
    j["email"] = email;
    // No se expone la contraseña
    j["active"] = active;
    j["created_at"] = createdAt;
    return j;
}

nlohmann::json User::toJsonWithToken() const {
    nlohmann::json j = toJson();
    
    if (authToken.has_value()) {
        j["token"] = authToken.value();
        
        if (tokenExpiry.has_value()) {
            auto expiry_time_t = std::chrono::system_clock::to_time_t(tokenExpiry.value());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&expiry_time_t), "%Y-%m-%d %H:%M:%S");
            j["token_expiry"] = ss.str();
        }
    }
    
    return j;
}

User User::fromJson(const nlohmann::json& json) {
    User user;
    if (json.contains("id")) {
        user.id = json["id"].get<int>();
    }
    if (json.contains("username")) {
        user.username = json["username"].get<std::string>();
    }
    if (json.contains("email")) {
        user.email = json["email"].get<std::string>();
    }
    if (json.contains("password")) {
        user.password = json["password"].get<std::string>();
    }
    if (json.contains("active")) {
        user.active = json["active"].get<bool>();
    }
    if (json.contains("created_at")) {
        user.createdAt = json["created_at"].get<std::string>();
    }
    return user;
}