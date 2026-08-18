#ifndef AREA_PROTEGIDA_REPOSITORY_HPP
#define AREA_PROTEGIDA_REPOSITORY_HPP

#include <optional>
#include <vector>

#include "../models/area_protegida.hpp"

struct AreaProtegidaFilters {
    std::optional<AreaProtegidaTipo> tipo;
    // Bbox del mapa, para pedir solo las áreas que se ven. Los cuatro van
    // juntos o ninguno.
    std::optional<double> min_lat;
    std::optional<double> min_lng;
    std::optional<double> max_lat;
    std::optional<double> max_lng;
};

class IAreaProtegidaRepository {
public:
    virtual ~IAreaProtegidaRepository() = default;

    virtual std::vector<AreaProtegida> find(const AreaProtegidaFilters& filters) = 0;
    virtual std::optional<AreaProtegida> findById(int id) = 0;
    virtual std::vector<EspecieEnArea> especiesEnArea(int areaId, int limit) = 0;
};

#endif // AREA_PROTEGIDA_REPOSITORY_HPP
