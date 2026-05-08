#ifndef POSTGRES_FAMILIA_REPOSITORY_HPP
#define POSTGRES_FAMILIA_REPOSITORY_HPP

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include "familia_repository.hpp"
#include "../utils/database.hpp"

class PostgresFamiliaRepository : public IFamiliaRepository
{
private:
    std::shared_ptr<Database> database;

    Familia mapRowToFamilia(const pqxx::row &row);

public:
    explicit PostgresFamiliaRepository(std::shared_ptr<Database> database);
    void initDatabase();
    std::vector<Familia> getAll() override;
    std::optional<Familia> findById(int id) override;
    std::optional<Familia> findByNombre(const std::string& nombre) override;
    Familia create(const Familia &familia) override;
    Familia update(const Familia &familia) override;
    bool remove(int id) override;
    std::vector<Imagen> getImagenes(int familia_id) override;
    bool setImagenPrincipal(int familia_id, const std::string& imagen_url) override;
    bool agregarImagen(int familia_id, const std::string& imagen_url, bool es_principal = false) override;
    bool eliminarImagen(int familia_id, const std::string& imagen_url) override;
};

#endif // POSTGRES_FAMILIA_REPOSITORY_HPP