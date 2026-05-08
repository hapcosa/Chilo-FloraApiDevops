#include "../../include/utils/database.hpp"
#include <iostream>

Database::Database(const std::string &connString) : connectionString(connString) {}

std::unique_ptr<pqxx::connection> Database::createConnection()
{
    try
    {
        return std::make_unique<pqxx::connection>(connectionString);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error de conexión a la base de datos: " << e.what() << std::endl;
        throw;
    }
}