#ifndef POSTGRES_INSIGNIA_REPOSITORY_HPP
#define POSTGRES_INSIGNIA_REPOSITORY_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "insignia_repository.hpp"

class PostgresInsigniaRepository : public IInsigniaRepository {
private:
    std::shared_ptr<Database> database;

    Insignia mapRowToInsignia(const pqxx::row& row);

public:
    explicit PostgresInsigniaRepository(std::shared_ptr<Database> database);

    std::vector<Insignia> findAll() override;
    std::optional<Insignia> findByCodigo(const std::string& codigo) override;
    std::vector<InsigniaOtorgada> findByUsuario(int usuarioId) override;
    std::map<int, std::vector<InsigniaOtorgada>> findByUsuarios(
        const std::vector<int>& usuarioIds) override;
    bool otorgar(int usuarioId, int insigniaId, int otorgadaPor,
                 const std::optional<std::string>& motivo) override;
    bool revocar(int usuarioId, int insigniaId) override;
    int recalcularAutomaticas() override;
};

#endif // POSTGRES_INSIGNIA_REPOSITORY_HPP
