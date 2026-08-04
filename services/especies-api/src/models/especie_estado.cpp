#include "../../include/models/especie_estado.hpp"
#include <stdexcept>

std::string especieEstadoToString(EspecieEstado estado) {
    switch (estado) {
        case EspecieEstado::Borrador:  return "borrador";
        case EspecieEstado::Publicada: return "publicada";
    }
    throw std::logic_error("EspecieEstado con valor inválido en switch");
}

EspecieEstado especieEstadoFromString(const std::string& s) {
    if (s == "borrador")  return EspecieEstado::Borrador;
    if (s == "publicada") return EspecieEstado::Publicada;
    throw std::invalid_argument("Estado de especie desconocido: '" + s + "'");
}

bool isValidEspecieEstado(const std::string& s) {
    return s == "borrador" || s == "publicada";
}
