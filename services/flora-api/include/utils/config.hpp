#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

class Config {
private:
    std::string apiHost;
    int apiPort;
    std::string dbHost;
    std::string dbPort;
    std::string dbName;
    std::string dbUser;
    std::string dbPassword;

public:
    Config();
    
    // Getters
    std::string getApiHost() const;
    int getApiPort() const;
    std::string getDbConnectionString() const;
    
    // Cargar configuración desde variables de entorno
    void loadFromEnvironment();
};

#endif // CONFIG_HPP