#include "../../include/utils/query_params.hpp"

#include <cctype>

namespace utils {

std::string percentDecode(const std::string& valor) {
    std::string salida;
    salida.reserve(valor.size());

    for (std::size_t i = 0; i < valor.size(); ++i) {
        const bool hayPar = i + 2 < valor.size() &&
                            std::isxdigit(static_cast<unsigned char>(valor[i + 1])) &&
                            std::isxdigit(static_cast<unsigned char>(valor[i + 2]));
        if (valor[i] == '%' && hayPar) {
            salida.push_back(static_cast<char>(
                std::stoi(valor.substr(i + 1, 2), nullptr, 16)));
            i += 2;
            continue;
        }
        salida.push_back(valor[i]);
    }

    return salida;
}

} // namespace utils
