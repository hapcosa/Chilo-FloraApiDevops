#ifndef ESPECIE_FILTERS_HPP
#define ESPECIE_FILTERS_HPP

#include <optional>
#include <string>
#include <vector>

#include "../models/especie.hpp"
#include "../models/reino.hpp"

// Filtros para listar especies. Todos opcionales: si no se setea, no se
// añade restricción. limit/offset gobiernan la paginación. Validación
// de límites razonables ocurre en EspecieService antes de llamar al repo.
struct EspecieFilters {
    std::optional<Reino> reino;
    std::optional<int>   genero_id;     // FK a generos
    std::optional<int>   familia_id;    // requiere JOIN con generos
    std::optional<std::string> conservacion;  // IUCN: LC, NT, VU, EN, CR, EW, EX, DD, NE
    std::optional<bool>  endemica;
    std::optional<std::string> q;       // búsqueda en nombre_comun + nombre_cientifico (ILIKE)

    // Paginación. Defaults razonables.
    int limit  = 50;
    int offset = 0;

    // Ordenamiento. Whitelist en el repo: nombre_cientifico, nombre_comun, created_at.
    std::string orderby  = "nombre_cientifico";
    std::string orderdir = "asc";  // "asc" | "desc"
};

// Resultado de una búsqueda paginada.
struct EspecieSearchResult {
    std::vector<Especie> data;
    int total = 0;
};

#endif  // ESPECIE_FILTERS_HPP
