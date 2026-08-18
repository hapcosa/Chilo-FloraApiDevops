#ifndef UTILS_FECHAS_HPP
#define UTILS_FECHAS_HPP

#include <ctime>
#include <string>

namespace utils {

// ¿Es `observado_en` una fecha en la que alguien pudo observar algo?
//
// Desde la Fase 9 un encuentro puede ser un recuerdo de hace años, así que ya
// no basta con "es de ahora". Pero sigue habiendo dos cosas que no pueden ser:
//
//   - El futuro. Nadie observó nada mañana; un `observado_en` futuro ordena mal
//     el feed y contamina cualquier serie temporal. Se toleran 24 h de margen
//     porque el reloj del teléfono se desfasa y la zona horaria del cliente no
//     tiene por qué coincidir con la del servidor.
//   - Antes de 1900. Es el piso: no hay fotografía de campo de un encuentro
//     anterior, y un año de dos dígitos mal parseado en el cliente cae ahí.
//
// Un valor sin forma de fecha se rechaza: mejor un 400 que una fila con basura.
bool observadoEnEsAceptable(const std::string& iso8601, std::time_t ahora);

} // namespace utils

#endif // UTILS_FECHAS_HPP
