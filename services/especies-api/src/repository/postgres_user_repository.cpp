#include "../../include/repository/postgres_user_repository.hpp"
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

PostgresUserRepository::PostgresUserRepository(const std::string& connectionString)
    : connectionString(connectionString) {}

std::unique_ptr<pqxx::connection> PostgresUserRepository::createConnection() const {
    try {
        return std::make_unique<pqxx::connection>(connectionString);
    } catch (const pqxx::broken_connection& e) {
        throw std::runtime_error("Failed to connect to database: " + std::string(e.what()));
    }
}

User PostgresUserRepository::rowToUser(const pqxx::row& row) const {
    User user;
    user.id = row["id"].as<int>();
    user.username = row["username"].as<std::string>();
    user.email = row["email"].as<std::string>();
    user.passwordHash = row["password_hash"].as<std::string>();
    user.firstName = row["first_name"].is_null() ? "" : row["first_name"].as<std::string>();
    user.lastName = row["last_name"].is_null() ? "" : row["last_name"].as<std::string>();
    user.createdAt = std::chrono::system_clock::from_time_t(row["created_at"].as<std::time_t>());
    user.updatedAt = std::chrono::system_clock::from_time_t(row["updated_at"].as<std::time_t>());
    
    if (!row["token"].is_null()) {
        user.token = row["token"].as<std::string>();
    }
    
    if (!row["token_expiry"].is_null()) {
        user.tokenExpiry = std::chrono::system_clock::from_time_t(row["token_expiry"].as<std::time_t>());
    }
    
    return user;
}

bool PostgresUserRepository::initializeSchema() {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        txn.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "    id SERIAL PRIMARY KEY,"
            "    username VARCHAR(100) UNIQUE NOT NULL,"
            "    email VARCHAR(255) UNIQUE NOT NULL,"
            "    password_hash VARCHAR(255) NOT NULL,"
            "    first_name VARCHAR(100),"
            "    last_name VARCHAR(100),"
            "    token VARCHAR(255) UNIQUE,"
            "    token_expiry TIMESTAMP,"
            "    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP"
            ");"
            
            "CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);"
            "CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);"
            "CREATE INDEX IF NOT EXISTS idx_users_token ON users(token);"
        );
        
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

User PostgresUserRepository::create(User user) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "INSERT INTO users (username, email, password_hash, first_name, last_name, created_at, updated_at) "
            "VALUES ($1, $2, $3, $4, $5, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP) "
            "RETURNING id, username, email, password_hash, first_name, last_name, token, token_expiry, "
            "extract(epoch from created_at) as created_at, extract(epoch from updated_at) as updated_at",
            user.username, user.email, user.passwordHash, user.firstName, user.lastName
        );
        
        txn.commit();
        
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to create user: " + std::string(e.what()));
    }
}

std::optional<User> PostgresUserRepository::findById(int id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "SELECT id, username, email, password_hash, first_name, last_name, token, token_expiry, "
            "extract(epoch from created_at) as created_at, extract(epoch from updated_at) as updated_at "
            "FROM users WHERE id = $1",
            id
        );
        
        txn.commit();
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find user by id: " + std::string(e.what()));
    }
}

std::optional<User> PostgresUserRepository::findByUsername(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "SELECT id, username, email, password_hash, first_name, last_name, token, token_expiry, "
            "extract(epoch from created_at) as created_at, extract(epoch from updated_at) as updated_at "
            "FROM users WHERE username = $1",
            username
        );
        
        txn.commit();
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find user by username: " + std::string(e.what()));
    }
}

std::optional<User> PostgresUserRepository::findByEmail(const std::string& email) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "SELECT id, username, email, password_hash, first_name, last_name, token, token_expiry, "
            "extract(epoch from created_at) as created_at, extract(epoch from updated_at) as updated_at "
            "FROM users WHERE email = $1",
            email
        );
        
        txn.commit();
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find user by email: " + std::string(e.what()));
    }
}

std::optional<User> PostgresUserRepository::findByToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "SELECT id, username, email, password_hash, first_name, last_name, token, token_expiry, "
            "extract(epoch from created_at) as created_at, extract(epoch from updated_at) as updated_at "
            "FROM users WHERE token = $1 AND (token_expiry IS NULL OR token_expiry > CURRENT_TIMESTAMP)",
            token
        );
        
        txn.commit();
        
        if (result.empty()) {
            return std::nullopt;
        }
        
        return rowToUser(result[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find user by token: " + std::string(e.what()));
    }
}

std::vector<User> PostgresUserRepository::findAll() {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec(
            "SELECT id, username, email, password_hash, first_name, last_name, token, token_expiry, "
            "extract(epoch from created_at) as created_at, extract(epoch from updated_at) as updated_at "
            "FROM users ORDER BY id"
        );
        
        txn.commit();
        
        std::vector<User> users;
        users.reserve(result.size());
        
        for (const auto& row : result) {
            users.push_back(rowToUser(row));
        }
        
        return users;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find all users: " + std::string(e.what()));
    }
}

bool PostgresUserRepository::update(const User& user) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "UPDATE users SET "
            "username = $1, "
            "email = $2, "
            "password_hash = $3, "
            "first_name = $4, "
            "last_name = $5, "
            "updated_at = CURRENT_TIMESTAMP "
            "WHERE id = $6 RETURNING id",
            user.username, user.email, user.passwordHash, user.firstName, user.lastName, user.id
        );
        
        txn.commit();
        
        return !result.empty();
    } catch (const std::exception& e) {
        return false;
    }
}

bool PostgresUserRepository::deleteById(int id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "DELETE FROM users WHERE id = $1 RETURNING id",
            id
        );
        
        txn.commit();
        
        return !result.empty();
    } catch (const std::exception& e) {
        return false;
    }
}

bool PostgresUserRepository::updateUserToken(int userId, const std::string& token,
                                         const std::chrono::system_clock::time_point& expiry) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        // Convert time_point to string in ISO format
        std::time_t expiryTime = std::chrono::system_clock::to_time_t(expiry);
        std::tm* tm = std::gmtime(&expiryTime);
        char buffer[30];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
        
        pqxx::result result = txn.exec_params(
            "UPDATE users SET "
            "token = $1, "
            "token_expiry = $2, "
            "updated_at = CURRENT_TIMESTAMP "
            "WHERE id = $3 RETURNING id",
            token, buffer, userId
        );
        
        txn.commit();
        
        return !result.empty();
    } catch (const std::exception& e) {
        return false;
    }
}

bool PostgresUserRepository::invalidateUserToken(int userId) {
    std::lock_guard<std::mutex> lock(mutex);
    
    try {
        auto conn = createConnection();
        pqxx::work txn(*conn);
        
        pqxx::result result = txn.exec_params(
            "UPDATE users SET "
            "token = NULL, "
            "token_expiry = NULL, "
            "updated_at = CURRENT_TIMESTAMP "
            "WHERE id = $1 RETURNING id",
            userId
        );
        
        txn.commit();
        
        return !result.empty();
    } catch (const std::exception& e) {
        return false;
    }
}