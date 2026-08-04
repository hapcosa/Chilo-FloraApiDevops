#ifndef ESPECIE_ESTADO_HPP
#define ESPECIE_ESTADO_HPP

#include <string>

// Estado editorial de una ficha. Espejo del tipo `especie_estado_enum`
// definido en migrations/0006_especies_estado.sql.
//
// Un borrador solo lo ve quien tiene curaduría sobre su categoría (y los
// roles globales). Es lo que impide que una ficha a medias se sincronice al
// cache SQLite del móvil, que se llena desde `GET /especies`.
enum class EspecieEstado {
    Borrador,
    Publicada
};

// Representación lowercase ("borrador", "publicada"): la que viaja en JSON y
// la que se inserta como literal del enum en Postgres.
std::string especieEstadoToString(EspecieEstado estado);

// Lanza std::invalid_argument si el valor no es uno de los dos estados.
EspecieEstado especieEstadoFromString(const std::string& s);

bool isValidEspecieEstado(const std::string& s);

#endif  // ESPECIE_ESTADO_HPP
