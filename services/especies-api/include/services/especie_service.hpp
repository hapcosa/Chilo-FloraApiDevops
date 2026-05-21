// ===== especie_service.hpp =====
#ifndef ESPECIE_SERVICE_HPP
#define ESPECIE_SERVICE_HPP

#include <vector>
#include <optional>
#include <memory>
#include "../models/especie.hpp"
#include "../repository/especie_repository.hpp"

class EspecieService {
private:
    std::shared_ptr<IEspecieRepository> repository;

    // Método privado para validación
    void validateEspecie(const Especie& especie);

public:
    explicit EspecieService(std::shared_ptr<IEspecieRepository> repo);

    // Métodos de consulta
    std::vector<Especie> getAllEspecies();
    std::vector<Especie> getEspeciesByReino(Reino reino);
    std::optional<Especie> getEspecieById(int id);
    std::optional<Especie> searchByNombreCientifico(const std::string& nombre);
    std::vector<Especie> searchByGenero(const std::string& genero);

    // Métodos CRUD
    Especie createEspecie(const Especie& especie);
    Especie updateEspecie(const Especie& especie);
    bool deleteEspecie(int id);

    // Métodos para manejo de imágenes
    std::string addImagenToEspecie(int especie_id, const std::string& image_data,bool es_principal);
    bool removeImagenFromEspecie(int especie_id, const std::string& image_url);
    bool setImagenPrincipal(int especie_id, const std::string& image_url);
    std::vector<std::string> getImagenesByEspecieId(int especie_id);
    std::string subirImagen(int especie_id, const std::vector<uint8_t>& imagen_data, bool es_principal);
};

#endif // ESPECIE_SERVICE_HPP
