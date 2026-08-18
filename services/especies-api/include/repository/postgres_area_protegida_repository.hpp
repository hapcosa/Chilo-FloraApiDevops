#ifndef POSTGRES_AREA_PROTEGIDA_REPOSITORY_HPP
#define POSTGRES_AREA_PROTEGIDA_REPOSITORY_HPP

#include <memory>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "area_protegida_repository.hpp"

class PostgresAreaProtegidaRepository : public IAreaProtegidaRepository {
private:
    std::shared_ptr<Database> database;

    AreaProtegida mapRowToArea(const pqxx::row& row);

public:
    explicit PostgresAreaProtegidaRepository(std::shared_ptr<Database> database);

    std::vector<AreaProtegida> find(const AreaProtegidaFilters& filters) override;
    std::optional<AreaProtegida> findById(int id) override;
    std::vector<EspecieEnArea> especiesEnArea(int areaId, int limit) override;
};

#endif // POSTGRES_AREA_PROTEGIDA_REPOSITORY_HPP
