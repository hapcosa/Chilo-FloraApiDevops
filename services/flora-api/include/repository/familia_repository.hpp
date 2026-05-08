#ifndef FAMILIA_REPOSITORY_HPP
#define FAMILIA_REPOSITORY_HPP

#include <vector>
#include <optional>
#include "../models/familia.hpp"
#include "../models/imagen.hpp"
// Interfaz del repositorio (patrón repository)
class IFamiliaRepository {
public:
    virtual ~IFamiliaRepository() = default;
    
    // Métodos CRUD
    virtual std::vector<Familia> getAll() = 0;
    virtual std::optional<Familia> findById(int id) = 0;
    virtual std::optional<Familia> findByNombre(const std::string& nombre) = 0;
    virtual Familia create(const Familia& Familia) = 0;
    virtual Familia update(const Familia& Familia) = 0;
    virtual bool remove(int id) = 0;
    virtual bool agregarImagen(int especie_id, const std::string& imagen_url, bool es_principal = false) = 0;
    virtual bool eliminarImagen(int especie_id, const std::string& imagen_url) = 0;
    virtual std::vector<Imagen> getImagenes(int familia_id) = 0;
    virtual bool setImagenPrincipal(int familia_id, const std::string &imagen_url) = 0;
   
};
#endif