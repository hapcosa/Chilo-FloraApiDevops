#ifndef POSTGRES_GENERO_REPOSITORY_HPP
#define POSTGRES_GENERO_REPOSITORY_HPP

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include "genero_repository.hpp"
#include "../utils/database.hpp"

class PostgresGeneroRepository : public IGeneroRepository
{
private:
    std::shared_ptr<Database> database;  // Changed from 'db' to 'database' for consistency

    Genero mapRowToGenero(const pqxx::row &row);  // Corrected method name from mapRowToEspecie

public:
    explicit PostgresGeneroRepository(std::shared_ptr<Database> database);

    // CRUD operations
    std::vector<Genero> getAll() override;
    std::optional<Genero> findById(int id) override;
    std::optional<Genero> findByNombre(int familia_id, const std::string& nombre) override;
    std::vector<Genero> getByFamilia(const std::string& familia) override;
    Genero create(const Genero &genero) override;
    Genero update(const Genero &genero) override;  // Changed return type to Genero
    bool remove(int id) override;
    std::vector<Imagen> getImagenes(int genero_id) override;
    bool setImagenPrincipal(int genero_id, const std::string& imagen_url) override;
    bool agregarImagen(int genero_id, const std::string& imagen_url, bool es_principal = false) override;
    bool eliminarImagen(int genero_id, const std::string& imagen_url) override;
};

#endif // POSTGRES_GENERO_REPOSITORY_HPP
