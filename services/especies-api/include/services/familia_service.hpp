#ifndef FAMILIA_SERVICE_HPP
#define FAMILIA_SERVICE_HPP

#include <vector>
#include <optional>
#include <memory>
#include "../models/familia.hpp"
#include "../repository/familia_repository.hpp"

class FamiliaService {
private:
    std::shared_ptr<IFamiliaRepository> repository;

    // Método privado para validación
    void validateFamilia(const Familia& familia);

public:
    explicit FamiliaService(std::shared_ptr<IFamiliaRepository> repo);

    // Métodos de consulta
    std::vector<Familia> getAllFamilias();
    std::optional<Familia> findFamiliaById(int id);
    std::optional<Familia> findByNombre(const std::string& nombre);

    // Métodos CRUD
    Familia createFamilia(const Familia& familia);
    Familia updateFamilia(const Familia& familia);
    bool deleteFamilia(int id);

    // Métodos para manejo de imágenes
    std::string addImagenToFamilia(int familia_id, const std::string& image_data,bool es_principal);
    bool removeImagenFromFamilia(int familia_id, const std::string& image_url);
    bool setImagenPrincipal(int familia_id, const std::string& image_url);
    std::vector<std::string> getImagenesByFamiliaId(int familia_id);
    std::string subirImagen(int familia_id, const std::vector<uint8_t>& imagen_data, bool es_principal);
};

#endif // FAMILIA_SERVICE_HPP