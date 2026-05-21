#ifndef POSTGRESQL_ESPECIE_REPOSITORY_HPP
#define POSTGRESQL_ESPECIE_REPOSITORY_HPP

#include <pqxx/pqxx>
#include <memory>
#include <string>
#include "especie_repository.hpp"
#include "../utils/database.hpp"

class PostgreSQLEspecieRepository : public IEspecieRepository {
private:
    std::shared_ptr<Database> database;
    
    // Método auxiliar para mapear un resultado de la BD a un objeto Especie
    Especie mapRowToEspecie(const pqxx::row& row);

public:
    explicit PostgreSQLEspecieRepository(std::shared_ptr<Database> database);

    // Implementación de los métodos de la interfaz
    EspecieSearchResult find(const EspecieFilters& filters) override;
    std::optional<Especie> findById(int id) override;
    std::optional<Especie> getByNombreCientifico(const std::string& nombre) override;
    Especie create(const Especie& especie) override;
    Especie update(const Especie& especie) override;
    bool remove(int id) override;
    std::vector<std::string> getImagenes(int especie_id) override;
    bool setImagenPrincipal(int especie_id, const std::string& imagen_url) override;
    bool agregarImagen(int especie_id, const std::string& imagen_url, bool es_principal = false) override;
    bool eliminarImagen(int especie_id, const std::string& imagen_url) override;
};

#endif // POSTGRESQL_ESPECIE_REPOSITORY_HPP