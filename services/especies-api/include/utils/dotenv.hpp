#ifndef DOTENV_H
#define DOTENV_H

#include <string>
#include <map>
#include <utility>

class DotEnv {
private:
    std::map<std::string, std::string> variables;

public:
    bool load(const std::string& filename);
    std::string get(const std::string& key, const std::string& defaultValue = "") const;
};

#endif // DOTENV_H