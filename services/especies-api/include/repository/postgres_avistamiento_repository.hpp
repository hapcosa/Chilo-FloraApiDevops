#ifndef POSTGRES_AVISTAMIENTO_REPOSITORY_HPP
#define POSTGRES_AVISTAMIENTO_REPOSITORY_HPP

#include <memory>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "avistamiento_repository.hpp"

class PostgresAvistamientoRepository : public IAvistamientoRepository {
private:
    std::shared_ptr<Database> database;

    // `withCount` solo cuando la consulta trajo la columna
    // `identificaciones_count`; los RETURNING no la traen.
    Avistamiento mapRowToAvistamiento(const pqxx::row& row, bool withCount = false);

public:
    explicit PostgresAvistamientoRepository(std::shared_ptr<Database> database);

    Avistamiento create(const Avistamiento& avistamiento) override;
    AvistamientoSearchResult find(const AvistamientoFilters& filters) override;
    std::optional<Avistamiento> findById(int id) override;
    Avistamiento moderate(int id, const ModeracionAvistamiento& moderacion) override;
    std::optional<Avistamiento> compartir(int id, int usuarioId) override;
    std::vector<CeldaMapa> mapa(const MapaFilters& filters) override;
};

#endif // POSTGRES_AVISTAMIENTO_REPOSITORY_HPP

