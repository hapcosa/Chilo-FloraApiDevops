#ifndef UTILS_TIMESTAMPS_HPP
#define UTILS_TIMESTAMPS_HPP

#include <optional>
#include <string>

namespace utils {

// Normaliza un TIMESTAMPTZ tal como lo entrega libpqxx
// ("2026-08-04 18:55:08.259598+00") al perfil ISO 8601 que entienden los
// clientes ("2026-08-04T18:55:08.259Z").
//
// La fracción se trunca a milisegundos: ECMAScript solo define tres dígitos y
// Hermes rechaza los seis de Postgres con `Invalid Date`.
//
// Si el valor no tiene forma de fecha y hora se devuelve intacto, para que un
// dato inesperado en la BD no se convierta en un timestamp inventado.
std::string toIso8601(const std::string& valor);

// Variante para columnas anulables. Nombre distinto a propósito: como
// sobrecarga, un literal de cadena sería ambiguo entre std::string y
// std::optional<std::string>.
std::optional<std::string> toIso8601Opt(const std::optional<std::string>& valor);

} // namespace utils

#endif // UTILS_TIMESTAMPS_HPP
