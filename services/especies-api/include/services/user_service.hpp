#pragma once
#include "../repository/user_repository.hpp"
#include "../utils/password_utils.hpp"
#include "../utils/tokenutils.hpp"
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <tuple>

class UserService {
private:
    UserRepository& repository;

public:
    explicit UserService(UserRepository& repository) : repository(repository) {}

    // Registro de usuario
    User registerUser(const std::string& username, const std::string& email, const std::string& password);
    
    // Login y token management
    std::tuple<User, std::string> loginUser(const std::string& username, const std::string& password);
    bool logoutUser(int userId);
    
    // Verificación de token
    std::optional<User> validateToken(const std::string& token);
    
    // Operaciones CRUD
    std::vector<User> getAllUsers();
    std::optional<User> getUserById(int id);
    std::optional<User> getUserByUsername(const std::string& username);
    bool updateUser(const User& user);
    bool deleteUser(int id);
    
    // Cambio de contraseña
    bool changePassword(int userId, const std::string& oldPassword, const std::string& newPassword);
};