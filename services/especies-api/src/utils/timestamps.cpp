#include "../../include/utils/timestamps.hpp"

namespace {

bool esDigito(char c) { return c >= '0' && c <= '9'; }

// Molde de los primeros 19 caracteres: 'd' = dígito, 's' = separador entre
// fecha y hora (espacio en Postgres, 'T' en ISO), el resto es literal.
constexpr const char* kMoldeFechaHora = "dddd-dd-ddsdd:dd:dd";

bool tieneFechaHora(const std::string& valor) {
    if (valor.size() < 19) return false;
    for (std::size_t i = 0; i < 19; ++i) {
        const char molde = kMoldeFechaHora[i];
        if (molde == 'd') {
            if (!esDigito(valor[i])) return false;
        } else if (molde == 's') {
            if (valor[i] != ' ' && valor[i] != 'T' && valor[i] != 't') return false;
        } else if (valor[i] != molde) {
            return false;
        }
    }
    return true;
}

} // namespace

namespace utils {

std::string toIso8601(const std::string& valor) {
    if (!tieneFechaHora(valor)) return valor;

    std::string salida = valor.substr(0, 19);
    salida[10] = 'T';

    std::size_t pos = 19;

    std::string milisegundos = "000";
    if (pos < valor.size() && valor[pos] == '.') {
        ++pos;
        const std::size_t inicio = pos;
        while (pos < valor.size() && esDigito(valor[pos])) ++pos;
        const std::string fraccion = valor.substr(inicio, pos - inicio);
        for (std::size_t i = 0; i < 3 && i < fraccion.size(); ++i) {
            milisegundos[i] = fraccion[i];
        }
    }
    salida += '.';
    salida += milisegundos;

    // Sin desplazamiento asumimos UTC: la columna es TIMESTAMPTZ y el servicio
    // corre con TimeZone=UTC, así que un valor sin zona ya está en UTC.
    const std::string resto = valor.substr(pos);
    if (resto.empty() || resto == "Z" || resto == "z") {
        salida += 'Z';
        return salida;
    }

    if ((resto[0] != '+' && resto[0] != '-') || resto.size() < 3
        || !esDigito(resto[1]) || !esDigito(resto[2])) {
        return valor;
    }

    const std::string horas = resto.substr(1, 2);
    std::string minutos = "00";
    std::size_t p = 3;
    if (p < resto.size() && resto[p] == ':') ++p;
    if (p + 1 < resto.size() && esDigito(resto[p]) && esDigito(resto[p + 1])) {
        minutos = resto.substr(p, 2);
    }
    // Los segundos del desplazamiento (zonas históricas) se descartan: ningún
    // cliente los acepta y ninguna zona vigente los usa.

    if (horas == "00" && minutos == "00") {
        salida += 'Z';
        return salida;
    }

    salida += resto[0];
    salida += horas;
    salida += ':';
    salida += minutos;
    return salida;
}

std::optional<std::string> toIso8601Opt(const std::optional<std::string>& valor) {
    if (!valor.has_value()) return std::nullopt;
    return toIso8601(*valor);
}

} // namespace utils
