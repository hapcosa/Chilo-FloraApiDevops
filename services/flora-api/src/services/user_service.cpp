#include "../../include/services/user_service.hpp"
#include <stdexcept>

User UserService::registerUser(const std::string& username, const std::string& email, const std::string& password) {
    // Validaciones básicas
    if (username.empty() || email.empty() || password.empty()) {
        throw std::runtime_error("Username, email, and password cannot be empty");
    }
    
    // Validar formato de email (simplificado)
    if (email.find('@') == std::string::npos) {
        throw std::runtime_error("Invalid email format");
    }
    
    // Validar longitud de contraseña
    if (password.length() < 8) {
        throw std::runtime_error("Password must be at least 8 characters long");
    }
    
    // Check if username already exists
    if (repository.findByUsername(username).has_value()) {
        throw std::runtime_error("Username already exists");
    }
    
    // Check if email already exists
    if (repository.findByEmail(email).has_value()) {
        throw std::runtime_error("Email already exists");
    }
    
    // Hash the password
    std::string hashedPassword = PasswordUtils::hashPassword(password);
    
    // Create user object
    User user(0, username, email, hashedPassword, true);
    
    // Persist user
    return repository.create(user);
}

std::tuple<User, std::string> UserService::loginUser(const std::string& username, const std::string& password) {
    // Find user by username
    auto user = repository.findByUsername(username);
    if (!user.has_value()) {
        throw std::runtime_error("Invalid username or password");
    }
    
    // Verify password
    if (!PasswordUtils::verifyPassword(password, user->getPassword())) {
        throw std::runtime_error("Invalid username or password");
    }
    
    // Generate JWT token
    std::string token = TokenUtils::generateJWT(user->getId(), user->getUsername());
    
    // Calculate token expiry (24 hours from now)
    auto now = std::chrono::system_clock::now();
    auto expiry = now + std::chrono::hours(24);
    
    // Update user with token
    User updatedUser = user.value();
    updatedUser.setAuthToken(token);
    updatedUser.setTokenExpiry(expiry);
    
    // Update in repository
    repository.update(updatedUser);
    
    return std::make_tuple(updatedUser, token);
}

bool UserService::logoutUser(int userId) {
    return repository.invalidateUserToken(userId);
}

std::optional<User> UserService::validateToken(const std::string& token) {
    int userId;
    if (!TokenUtils::validateJWT(token, userId)) {
        return std::nullopt;
    }
    
    auto user = repository.findById(userId);
    if (!user.has_value() || !user->hasValidToken()) {
        return std::nullopt;
    }
    
    return user;
}

std::vector<User> UserService::getAllUsers() {
    return repository.findAll();
}

std::optional<User> UserService::getUserById(int id) {
    return repository.findById(id);
}

std::optional<User> UserService::getUserByUsername(const std::string& username) {
    return repository.findByUsername(username);
}

bool UserService::updateUser(const User& user) {
    return repository.update(user);
}

bool UserService::deleteUser(int id) {
    return repository.deleteById(id);
}

bool UserService::changePassword(int userId, const std::string& oldPassword, const std::string& newPassword) {
    auto user = repository.findById(userId);
    if (!user.has_value()) {
        return false;
    }
    
    // Verify old password
    if (!PasswordUtils::verifyPassword(oldPassword, user->getPassword())) {
        return false;
    }
    
    // Validate new password
    if (newPassword.length() < 8) {
        throw std::runtime_error("New password must be at least 8 characters long");
    }
    
    // Hash the new password
    std::string hashedPassword = PasswordUtils::hashPassword(newPassword);
    
    // Update user
    User updatedUser = user.value();
    updatedUser.setPassword(hashedPassword);
    
    return repository.update(updatedUser);
}