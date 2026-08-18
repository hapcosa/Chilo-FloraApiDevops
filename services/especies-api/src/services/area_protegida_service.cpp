#include "../../include/services/area_protegida_service.hpp"

#include <stdexcept>
#include <utility>

namespace {

// La ficha de un parque muestra una lista, no un catálogo: pasado cierto punto
// el visitante deja de leer y la consulta empieza a doler.
constexpr int kLimiteEspeciesPorDefecto = 50;
constexpr int kLimiteEspeciesMaximo = 200;

} // namespace

AreaProtegidaService::AreaProtegidaService(
    std::shared_ptr<IAreaProtegidaRepository> repository)
    : repository(std::move(repository)) {}

std::vector<AreaProtegida> AreaProtegidaService::listar(
    const AreaProtegidaFilters& filters) {
    const bool algunoDelBbox = filters.min_lat || filters.min_lng ||
                               filters.max_lat || filters.max_lng;
    const bool todosDelBbox = filters.min_lat && filters.min_lng &&
                              filters.max_lat && filters.max_lng;
    if (algunoDelBbox && !todosDelBbox) {
        throw std::invalid_argument("el bbox necesita sus cuatro coordenadas");
    }
    if (todosDelBbox) {
        if (*filters.min_lat > *filters.max_lat || *filters.min_lng > *filters.max_lng) {
            throw std::invalid_argument("bbox invertido");
        }
        if (*filters.min_lat < -90 || *filters.max_lat > 90 ||
            *filters.min_lng < -180 || *filters.max_lng > 180) {
            throw std::invalid_argument("bbox fuera de rango");
        }
    }
    return repository->find(filters);
}

std::optional<AreaProtegida> AreaProtegidaService::obtener(int id) {
    return repository->findById(id);
}

std::vector<EspecieEnArea> AreaProtegidaService::especiesEnArea(int areaId, int limit) {
    if (limit <= 0) {
        limit = kLimiteEspeciesPorDefecto;
    }
    if (limit > kLimiteEspeciesMaximo) {
        limit = kLimiteEspeciesMaximo;
    }
    return repository->especiesEnArea(areaId, limit);
}
