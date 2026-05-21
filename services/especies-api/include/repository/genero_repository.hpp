#ifndef GENERO_REPOSITORY_HPP
#define GENERO_REPOSITORY_HPP
#include <vector>
#include <optional>
#include "../models/genero.hpp"
#include "../models/imagen.hpp"

// Interfaz del repositorio (patrón repository)
class IGeneroRepository {
public:
    virtual ~IGeneroRepository() = default;

    // Métodos CRUD
    virtual std::vector<Genero> getAll() = 0;
    virtual std::optional<Genero> findById(int id) = 0;
    // findByNombre toma familia_id porque (familia_id, nombre) es la clave única
    // tras la migración 0002_multi_reino.
    virtual std::optional<Genero> findByNombre(int familia_id, const std::string& nombre) = 0;
    virtual std::vector<Genero> getByFamilia(const std::string& familia) = 0;
    virtual Genero create(const Genero& genero) = 0;
    virtual Genero update(const Genero& genero) = 0;
    virtual bool remove(int id) = 0;
    virtual bool agregarImagen(int especie_id, const std::string& imagen_url, bool es_principal = false) = 0;
    virtual bool eliminarImagen(int especie_id, const std::string& imagen_url) = 0;
    virtual std::vector<Imagen> getImagenes(int genero_id) = 0;
    virtual bool setImagenPrincipal(int genero_id, const std::string &imagen_url) = 0;
};
#endif
