#include "../../include/utils/conservacion.hpp"

#include <array>
#include <regex>

namespace {

// Códigos UICN de riesgo y los nombres con que aparecen escritos en español.
// "PELIGRO" cubre tanto "En Peligro" como "En Peligro Crítico"; "EN" ya cubre
// el primero, pero no toda ficha usa el código.
constexpr std::array<const char*, 6> kTokensSensibles{
    "VU", "EN", "CR", "EW", "VULNERABLE", "PELIGRO"};

// Delimitado por algo que no sea letra: sin esto "EN" haría sensible a
// cualquier ficha que dijera "endémica" y "CR" a una que dijera "críptica".
std::string construirPatron() {
    std::string alternativas;
    for (const auto* token : kTokensSensibles) {
        if (!alternativas.empty()) alternativas += "|";
        alternativas += token;
    }
    return "(^|[^[:alpha:]])(" + alternativas + ")([^[:alpha:]]|$)";
}

} // namespace

namespace utils {

bool esEstadoConservacionSensible(const std::string& estado) {
    static const std::regex patron(construirPatron(),
                                   std::regex::icase | std::regex::extended);
    return std::regex_search(estado, patron);
}

std::string patronSqlEstadoSensible() {
    return construirPatron();
}

} // namespace utils
