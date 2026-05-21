#ifndef REINO_HPP
#define REINO_HPP

#include <string>

// Reinos soportados. Espejo del tipo enum `reino_enum` definido en
// services/especies-api/migrations/0002_multi_reino.sql.
//
// Serialización: lowercase ASCII ("animalia", "plantae", ...). Es el valor
// que viaja en JSON externo y el que se insertará tal cual como literal
// `reino_enum` en Postgres (cast implícito desde TEXT).
enum class Reino {
    Animalia,
    Plantae,
    Fungi,
    Protista,
    Monera
};

// Devuelve la representación lowercase del reino ("animalia", etc.).
std::string reinoToString(Reino reino);

// Parsea un string lowercase a Reino. Lanza std::invalid_argument si el
// valor no corresponde a ninguno de los 5 reinos.
Reino reinoFromString(const std::string& s);

// Validador puro sin lanzar. Útil cuando se viene de input externo.
bool isValidReino(const std::string& s);

#endif // REINO_HPP
