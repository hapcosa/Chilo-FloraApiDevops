#pragma once
#include "../repository/user_repository.hpp"
#include <pqxx/pqxx>
#include <mutex>
#include <string>

class PostgresUserRepository : public UserRepository {
private:
    std::string connectionString;
    std::mutex mutex; // Para operaciones thread-safe

    // Helper method to create a connection
    std::unique_ptr<pqxx::connection> createConnection() const;
    
    // Helper method to convert a database row to a User object
    User rowToUser(const pqxx::row& row) const;

public:
    explicit PostgresUserRepository(const std::string& connectionString);
    
    // Initialize the database schema
    bool initializeSchema();

    // Implementation of UserRepository interface
    User create(User user) override;
    std::optional<User> findById(int id) override;
    std::optional<User> findByUsername(const std::string& username) override;
    std::optional<User> findByEmail(const std::string& email) override;
    std::optional<User> findByToken(const std::string& token) override;
    std::vector<User> findAll() override;
    bool update(const User& user) override;
    bool deleteById(int id) override;
    bool updateUserToken(int userId, const std::string& token, 
                        const std::chrono::system_clock::time_point& expiry) override;
    bool invalidateUserToken(int userId) override;
};