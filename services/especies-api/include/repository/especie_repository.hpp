#ifndef ESPECIE_REPOSITORY_HPP
#define ESPECIE_REPOSITORY_HPP

#include <vector>
#include <optional>
#include "../models/especie.hpp"

// Interfaz del repositorio (patrón repository)
class IEspecieRepository {
public:
    virtual ~IEspecieRepository() = default;
    
    // Métodos CRUD
    virtual std::vector<Especie> getAll() = 0;
    virtual std::vector<Especie> getByReino(Reino reino) = 0;
    virtual std::optional<Especie> findById(int id) = 0;
    virtual std::optional<Especie> getByNombreCientifico(const std::string& nombre) = 0;
    virtual std::vector<Especie> getByGenero(const std::string& Genero) = 0;
    virtual Especie create(const Especie& especie) = 0;
    virtual Especie update(const Especie& especie) = 0;
    virtual bool remove(int id) = 0;
    virtual bool agregarImagen(int especie_id, const std::string& imagen_url, bool es_principal = false) = 0;
    virtual bool eliminarImagen(int especie_id, const std::string& imagen_url) = 0;
    virtual std::vector<std::string> getImagenes(int especie_id) = 0;
    virtual bool setImagenPrincipal(int especie_id, const std::string& imagen_url) = 0;

};

#endif // ESPECIE_REPOSITORY_HPP