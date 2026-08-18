#ifndef UTILS_CONSERVACION_HPP
#define UTILS_CONSERVACION_HPP

#include <string>

namespace utils {

// ¿La ubicación pública de esta especie hay que difuminarla?
//
// `especies.estado_conservacion` es texto libre —el panel sugiere "LC, NT, VU,
// EN, CR…" pero no lo obliga—, así que la clasificación se hace por tokens y
// no por igualdad. Se consideran de riesgo las categorías UICN VU, EN, CR y EW,
// y sus nombres en español, porque publicar el punto exacto de una especie
// amenazada es una invitación al tráfico y a la presión de observadores. Es el
// mismo criterio que aplican GBIF e iNaturalist.
//
// Un estado vacío o desconocido **no** se considera sensible: ofuscar todo lo
// que no reconocemos dejaría el mapa entero en celdas de un kilómetro, que es
// tanto como no tener mapa. La contrapartida es que una ficha sin curar no
// protege a su especie; se mitiga en curaduría, llenando el campo.
bool esEstadoConservacionSensible(const std::string& estado);

// La misma regla, en la sintaxis de expresiones regulares de Postgres, para
// clasificar dentro de la consulta agregada. Sale de la misma lista de tokens
// que la función de arriba: son dos formas de escribir una sola regla, no dos
// reglas.
std::string patronSqlEstadoSensible();

} // namespace utils

#endif // UTILS_CONSERVACION_HPP
