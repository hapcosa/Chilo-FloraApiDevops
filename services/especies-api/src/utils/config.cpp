#include "../../include/utils/config.hpp"
#include <cstdlib>
#include <string>
Config::Config()
    : apiHost("0.0.0.0"), apiPort(9080),
      dbHost("postgres"), dbPort("5432"),
      dbName("chiloe_flora_db"), dbUser("postgres"), dbPassword("postgres")
{
    loadFromEnvironment();
}

std::string Config::getApiHost() const
{
    return apiHost;
}

int Config::getApiPort() const
{
    return apiPort;
}

std::string Config::getDbConnectionString() const
{
    return "host=" + dbHost + " port=" + dbPort +
           " dbname=" + dbName + " user=" + dbUser +
           " password=" + dbPassword;
}

void Config::loadFromEnvironment()
{
    // Cargar configuración de la API desde variables de entorno
    if (const char *env_api_host = std::getenv("API_HOST"))
    {
        apiHost = env_api_host;
    }

    if (const char *env_api_port = std::getenv("API_PORT"))
    {
        try
        {
            apiPort = std::stoi(env_api_port);
        }
        catch (...)
        {
            // Mantener el valor por defecto si hay error
        }
    }

    // Cargar configuración de la base de datos desde variables de entorno
    if (const char *env_db_host = std::getenv("DB_HOST"))
    {
        dbHost = env_db_host;
    }

    if (const char *env_db_port = std::getenv("DB_PORT"))
    {
        dbPort = env_db_port;
    }

    if (const char *env_db_name = std::getenv("DB_NAME"))
    {
        dbName = env_db_name;
    }

    if (const char *env_db_user = std::getenv("DB_USER"))
    {
        dbUser = env_db_user;
    }

    if (const char *env_db_password = std::getenv("DB_PASSWORD"))
    {
        dbPassword = env_db_password;
    }
}