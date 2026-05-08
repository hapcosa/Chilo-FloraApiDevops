#ifndef GENERO_SERVICE_HPP
#define GENERO_SERVICE_HPP

#include <vector>
#include <optional>
#include <memory>
#include "../models/genero.hpp"
#include "../repository/genero_repository.hpp"

class GeneroService
{
private:
    std::shared_ptr<IGeneroRepository> repository;
    void validateGenero(const Genero& genero);
public:
    explicit GeneroService(std::shared_ptr<IGeneroRepository> repo);

    // Métodos de negocio
    std::vector<Genero> getAllGeneros();
    std::optional<Genero> getGeneroById(int id);
    std::optional<Genero> findByNombre(const std::string &nombre);
    std::vector<Genero> searchByFamilia(const std::string &familia);
    Genero createGenero(const Genero &Genero);
    Genero updateGenero(const Genero &Genero);
    bool deleteGenero(int id);

    std::string addImagenToGenero(int genero_id, const std::string& image_data,bool es_principal);
    bool removeImagenFromGenero(int genero_id, const std::string& image_url);
    bool setImagenPrincipal(int genero_id, const std::string& image_url);
    std::vector<std::string> getImagenesByGeneroId(int genero_id);
    std::string subirImagen(int genero_id, const std::vector<uint8_t>& imagen_data, bool es_principal);
};

#endif
