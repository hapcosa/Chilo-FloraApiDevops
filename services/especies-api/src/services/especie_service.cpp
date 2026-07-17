#include "../../include/services/especie_service.hpp"
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <set>
#include <utility>

// Constructor
EspecieService::EspecieService(std::shared_ptr<IEspecieRepository> repo,
                                std::shared_ptr<AtributosSchemaValidator> validator,
                                std::shared_ptr<UploadPresignService> storageService)
        : repository(std::move(repo)),
          schemaValidator(std::move(validator)),
          storageService(std::move(storageService)) {}

// Validación: invariantes locales + atributos_especificos por reino.
void EspecieService::validateEspecie(const Especie& especie) {
    if (especie.getNombreCientifico().empty()) {
        throw std::invalid_argument("El nombre científico no puede estar vacío");
    }
    if (especie.getGeneroId() <= 0) {
        throw std::invalid_argument("'genero_id' debe ser > 0");
    }
    // Validación de atributos por reino contra JSON Schema. Lanza
    // std::invalid_argument con un mensaje detallado si no cumple.
    schemaValidator->validate(especie.getReino(),
                              especie.getAtributosEspecificos());
}

void EspecieService::validateFotoKeyExists(const std::string& key) const {
    const std::string& bucket = storageService->getConfig().especiesBucket;
    if (!storageService->objectExists(bucket, key)) {
        throw std::invalid_argument(
            "La key de foto no existe en object storage: " + key);
    }
}

// Métodos de consulta
EspecieSearchResult EspecieService::searchEspecies(const EspecieFilters& filters) {
    return repository->find(filters);
}

std::optional<Especie> EspecieService::getEspecieById(int id) {
    if (id <= 0) {
        throw std::invalid_argument("ID debe ser mayor que 0");
    }
    return repository->findById(id);
}

std::optional<Especie> EspecieService::searchByNombreCientifico(const std::string& nombre) {
    if (nombre.empty()) {
        throw std::invalid_argument("El nombre científico no puede estar vacío");
    }
    return repository->getByNombreCientifico(nombre);
}

Especie EspecieService::createEspecie(const Especie& especie) {
    validateEspecie(especie);

    // Verificar que no exista una especie con el mismo nombre científico
    if (repository->getByNombreCientifico(especie.getNombreCientifico())) {
        throw std::runtime_error("Ya existe una especie con ese nombre científico");
    }

    return repository->create(especie);
}

Especie EspecieService::updateEspecie(const Especie& especie) {
    if (especie.getId() <= 0) {
        throw std::invalid_argument("ID debe ser mayor que 0");
    }

    if (!repository->findById(especie.getId())) {
        throw std::runtime_error("Especie no encontrada");
    }

    validateEspecie(especie);

    // Verificar que no exista otra especie con el mismo nombre científico
    auto existing = repository->getByNombreCientifico(especie.getNombreCientifico());
    if (existing && existing->getId() != especie.getId()) {
        throw std::runtime_error("Ya existe otra especie con ese nombre científico");
    }

    return repository->update(especie);
}

Especie EspecieService::updateFotoKeys(int id, const FotoKeysUpdate& update) {
    if (id <= 0) {
        throw std::invalid_argument("ID debe ser mayor que 0");
    }
    if (!update.updateFotoPortadaKey && !update.updateFotosKeys) {
        throw std::invalid_argument("No hay campos de fotos para actualizar");
    }

    auto existing = repository->findById(id);
    if (!existing) {
        throw std::runtime_error("Especie no encontrada");
    }

    Especie updated = *existing;
    if (update.updateFotoPortadaKey) {
        if (update.fotoPortadaKey) {
            validateFotoKeyExists(*update.fotoPortadaKey);
        }
        updated.setFotoPortadaKey(update.fotoPortadaKey);
    }

    if (update.updateFotosKeys) {
        std::set<std::string> uniqueKeys;
        for (const auto& key : update.fotosKeys) {
            if (!uniqueKeys.insert(key).second) {
                throw std::invalid_argument("fotos_keys contiene keys duplicadas");
            }
            validateFotoKeyExists(key);
        }
        updated.setFotosKeys(update.fotosKeys);
    }

    validateEspecie(updated);
    return repository->update(updated);
}

bool EspecieService::deleteEspecie(int id) {
    if (id <= 0) {
        throw std::invalid_argument("ID debe ser mayor que 0");
    }

    if (!repository->findById(id)) {
        throw std::runtime_error("Especie no encontrada");
    }

    return repository->remove(id);
}

// Métodos para imágenes
std::string EspecieService::addImagenToEspecie(int especie_id, const std::string& image_data, bool es_principal) {
    if (especie_id <= 0) {
        throw std::invalid_argument("ID de especie debe ser mayor que 0");
    }

    if (image_data.empty()) {
        throw std::invalid_argument("Los datos de la imagen no pueden estar vacíos");
    }

    // 1. Verificar que la especie existe
    if (!repository->findById(especie_id)) {
        throw std::runtime_error("Especie no encontrada");
    }

    // 2. Crear directorio si no existe
    std::string dir_path = "./static/images/especies/";
    if (!std::filesystem::exists(dir_path)) {
        std::filesystem::create_directories(dir_path);
    }

    // 3. Generar nombre único y guardar imagen
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string filename = "especie_" + std::to_string(especie_id) + "_" + std::to_string(timestamp) + ".jpg";
    std::string image_path = dir_path + filename;

    try {
        std::ofstream file(image_path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("No se pudo crear el archivo de imagen");
        }
        file.write(image_data.data(), image_data.size());
        file.close();

        // 4. Guardar referencia en la base de datos
        std::string public_url = "/api/images/especies/" + filename;
        if (!repository->agregarImagen(especie_id, public_url,false)) {
            std::filesystem::remove(image_path); // Revertir si falla la BD
            throw std::runtime_error("Error al guardar imagen en la base de datos");
        }

        return public_url;
    } catch (const std::exception& e) {
        std::filesystem::remove(image_path); // Limpiar en caso de error
        throw;
    }
}

bool EspecieService::removeImagenFromEspecie(int especie_id, const std::string& image_url) {
    if (especie_id <= 0) {
        throw std::invalid_argument("ID de especie debe ser mayor que 0");
    }

    if (image_url.empty()) {
        throw std::invalid_argument("URL de imagen no puede estar vacía");
    }

    // 1. Extraer el nombre del archivo de la URL
    std::string filename = image_url.substr(image_url.find_last_of('/') + 1);
    std::string filepath = "./static/images/especie/" + filename; // RUTA CORREGIDA

    // 2. Eliminar de la base de datos primero
    if (repository->eliminarImagen(especie_id, image_url)) {
        // 3. Si éxito en BD, eliminar el archivo
        std::filesystem::remove(filepath);
        return true;
    }
    return false;
}

bool EspecieService::setImagenPrincipal(int especie_id, const std::string& image_url) {
    if (especie_id <= 0) {
        throw std::invalid_argument("ID de especie debe ser mayor que 0");
    }

    if (image_url.empty()) {
        throw std::invalid_argument("URL de imagen no puede estar vacía");
    }

    // Verificar que la especie existe
    if (!repository->findById(especie_id)) {
        throw std::runtime_error("Especie no encontrada");
    }

    return repository->setImagenPrincipal(especie_id, image_url);
}

std::vector<std::string> EspecieService::getImagenesByEspecieId(int especie_id) {
    if (especie_id <= 0) {
        throw std::invalid_argument("ID de especie debe ser mayor que 0");
    }

    return repository->getImagenes(especie_id);
}

// Implementación del método que faltaba en el header
std::string EspecieService::subirImagen(int especie_id, const std::vector<uint8_t>& imagen_data,bool es_principal) {
    if (imagen_data.empty()) {
        throw std::invalid_argument("Los datos de la imagen no pueden estar vacíos");
    }

    // Convertir vector<uint8_t> a string para usar con addImagenToEspecie
    std::string image_data(imagen_data.begin(), imagen_data.end());
    return addImagenToEspecie(especie_id, image_data,es_principal);
}
