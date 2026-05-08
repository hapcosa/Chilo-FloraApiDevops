#include "../../include/utils/dotenv.hpp"
#include <iostream>
#include <fstream>

bool DotEnv::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignorar líneas vacías o comentarios
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Buscar el signo igual
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            variables[key] = value;
        }
    }
    return true;
}

std::string DotEnv::get(const std::string& key, const std::string& defaultValue) const {
    auto it = variables.find(key);
    if (it != variables.end()) {
        return it->second;
    }
    return defaultValue;
}
