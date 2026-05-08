#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <pqxx/pqxx>
#include <memory>
#include <string>

class Database {
private:
    std::string connectionString;

public:
    explicit Database(const std::string& connString);
    
    // Método para crear una conexión a la base de datos
    std::unique_ptr<pqxx::connection> createConnection();
};

#endif // DATABASE_HPP