#include "../../include/models/reino.hpp"
#include <stdexcept>

std::string reinoToString(Reino reino) {
    switch (reino) {
        case Reino::Animalia: return "animalia";
        case Reino::Plantae:  return "plantae";
        case Reino::Fungi:    return "fungi";
        case Reino::Protista: return "protista";
        case Reino::Monera:   return "monera";
    }
    // No debería llegar aquí salvo corrupción de memoria; el switch cubre
    // todos los enumeradores y enum class no permite valores arbitrarios.
    throw std::logic_error("Reino con valor inválido en switch");
}

Reino reinoFromString(const std::string& s) {
    if (s == "animalia") return Reino::Animalia;
    if (s == "plantae")  return Reino::Plantae;
    if (s == "fungi")    return Reino::Fungi;
    if (s == "protista") return Reino::Protista;
    if (s == "monera")   return Reino::Monera;
    throw std::invalid_argument("Reino desconocido: '" + s + "'");
}

bool isValidReino(const std::string& s) {
    return s == "animalia" || s == "plantae" || s == "fungi"
        || s == "protista" || s == "monera";
}
