#pragma once
#include "../models/user.hpp"
#include <vector>
#include <memory>
#include <optional>
#include <string>
#include <chrono>

// Interfaz base para repositorio de usuarios
class UserRepository {
public:
    virtual ~UserRepository() = default;
    
    // Create
    virtual User create(User user) = 0;
    
    // Read
    virtual std::optional<User> findById(int id) = 0;
    virtual std::optional<User> findByUsername(const std::string& username) = 0;
    virtual std::optional<User> findByEmail(const std::string& email) = 0;
    virtual std::optional<User> findByToken(const std::string& token) = 0;
    virtual std::vector<User> findAll() = 0;
    
    // Update
    virtual bool update(const User& user) = 0;
    
    // Delete
    virtual bool deleteById(int id) = 0;
    
    // Token management
    virtual bool updateUserToken(int userId, const std::string& token, 
                         const std::chrono::system_clock::time_point& expiry) = 0;
    virtual bool invalidateUserToken(int userId) = 0;
};