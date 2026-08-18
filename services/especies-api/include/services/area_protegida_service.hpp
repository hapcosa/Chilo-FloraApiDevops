#ifndef AREA_PROTEGIDA_SERVICE_HPP
#define AREA_PROTEGIDA_SERVICE_HPP

#include <memory>
#include <optional>
#include <vector>

#include "../repository/area_protegida_repository.hpp"

class AreaProtegidaService {
private:
    std::shared_ptr<IAreaProtegidaRepository> repository;

public:
    explicit AreaProtegidaService(std::shared_ptr<IAreaProtegidaRepository> repository);

    std::vector<AreaProtegida> listar(const AreaProtegidaFilters& filters);
    std::optional<AreaProtegida> obtener(int id);
    std::vector<EspecieEnArea> especiesEnArea(int areaId, int limit);
};

#endif // AREA_PROTEGIDA_SERVICE_HPP
