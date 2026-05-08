#include "../../include/services/familia_service.hpp"
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iostream>
// Constructor
FamiliaService::FamiliaService(std::shared_ptr<IFamiliaRepository> repo)
        : repository(repo) {}

// Validación
void FamiliaService::validateFamilia(const Familia& familia) {
    if (familia.getNombre().empty()) {
        throw std::invalid_argument("El nombre de la familia no puede estar vacío");
    }if(familia.getDescripcion().empty()){
        throw  std::invalid_argument("La descripcion no puede estar vacía");
    }
    // Puedes añadir más validaciones según tus requisitos
}

// Métodos de consulta
std::vector<Familia> FamiliaService::getAllFamilias() {
    return repository->getAll();
}

std::optional<Familia> FamiliaService::findFamiliaById(int id) {
    if (id <= 0) {
        throw std::invalid_argument("ID debe ser mayor que 0");
    }
    return repository->findById(id);
}

std::optional<Familia> FamiliaService::findByNombre(const std::string& nombre) {
    if (nombre.empty()) {
        throw std::invalid_argument("El nombre no puede estar vacío");
    }
    return repository->findByNombre(nombre);
}

// Métodos CRUD
Familia FamiliaService::createFamilia(const Familia& familia) {
    validateFamilia(familia);

    // Verificar que no exista una familia con el mismo nombre
    if (repository->findByNombre(familia.getNombre())) {
        throw std::runtime_error("Ya existe una familia con ese nombre");
    }

    return repository->create(familia);
}

Familia FamiliaService::updateFamilia(const Familia& familia) {
    if (familia.getId() <= 0) {
        throw std::invalid_argument("ID debe ser mayor que 0");
    }

    if (!repository->findById(familia.getId())) {
        throw std::runtime_error("Familia no encontrada");
    }

    validateFamilia(familia);

    // Verificar que no exista otra familia con el mismo nombre
    auto existing = repository->findByNombre(familia.getNombre());
    if (existing && existing->getId() != familia.getId()) {
        throw std::runtime_error("Ya existe otra familia con ese nombre");
    }

    return repository->update(familia);
}

bool FamiliaService::deleteFamilia(int id) {
    if (id <= 0) {
        throw std::invalid_argument("ID debe ser mayor que 0");
    }

    if (!repository->findById(id)) {
        throw std::runtime_error("Familia no encontrada");
    }

    return repository->remove(id);
}

// Métodos para imágenes
// En familia_service.cpp, modifica el método addImagenToFamilia

std::string FamiliaService::addImagenToFamilia(int familia_id, const std::string& image_data,bool es_principal) {
    if (familia_id <= 0) {
        throw std::invalid_argument("ID de familia debe ser mayor que 0");
    }

    if (image_data.empty()) {
        throw std::invalid_argument("Los datos de la imagen no pueden estar vacíos");
    }

    // 1. Verificar que la familia existe
    if (!repository->findById(familia_id)) {
        throw std::runtime_error("Familia no encontrada");
    }

    // 2. Crear directorio si no existe - RUTA CORREGIDA
    std::string dir_path = "./static/images/familias/";
    if (!std::filesystem::exists(dir_path)) {
        std::filesystem::create_directories(dir_path);
    }

    // 3. Generar nombre único y guardar imagen
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    std::string filename = "familia_" + std::to_string(familia_id) + "_" + std::to_string(timestamp) + ".jpg";
    std::string image_path = dir_path + filename;

    try {
        std::ofstream file(image_path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("No se pudo crear el archivo de imagen en: " + image_path);
        }
        file.write(image_data.data(), image_data.size());
        file.close();

        // Verificar que el archivo se creó correctamente
        if (!std::filesystem::exists(image_path)) {
            throw std::runtime_error("El archivo no se guardó correctamente en: " + image_path);
        }

        // 4. Guardar referencia en la base de datos
        std::string public_url = "/api/images/familias/" + filename;
        if (!repository->agregarImagen(familia_id, public_url, es_principal)) {
            std::filesystem::remove(image_path); // Revertir si falla la BD
            throw std::runtime_error("Error al guardar imagen en la base de datos para familia: " + std::to_string(familia_id));
        }

        std::cout << "Imagen guardada exitosamente en: " << image_path << std::endl;
        return public_url;
    } catch (const std::exception& e) {
        std::cout << "Error al guardar imagen: " << e.what() << std::endl;
        std::filesystem::remove(image_path); // Limpiar en caso de error
        throw;
    }
}

// También corrige el método removeImagenFromFamilia
bool FamiliaService::removeImagenFromFamilia(int familia_id, const std::string& image_url) {
    if (familia_id <= 0) {
        throw std::invalid_argument("ID de familia debe ser mayor que 0");
    }

    if (image_url.empty()) {
        throw std::invalid_argument("URL de imagen no puede estar vacía");
    }

    // 1. Extraer el nombre del archivo de la URL
    std::string filename = image_url.substr(image_url.find_last_of('/') + 1);
    std::string filepath = "./static/images/familias/" + filename; // RUTA CORREGIDA

    // 2. Eliminar de la base de datos primero
    if (repository->eliminarImagen(familia_id, image_url)) {
        // 3. Si éxito en BD, eliminar el archivo
        std::filesystem::remove(filepath);
        return true;
    }
    return false;
}
bool FamiliaService::setImagenPrincipal(int familia_id, const std::string& image_url) {
    if (familia_id <= 0) {
        throw std::invalid_argument("ID de familia debe ser mayor que 0");
    }

    if (image_url.empty()) {
        throw std::invalid_argument("URL de imagen no puede estar vacía");
    }

    // Verificar que la familia existe
    if (!repository->findById(familia_id)) {
        throw std::runtime_error("Familia no encontrada");
    }

    return repository->setImagenPrincipal(familia_id, image_url);
}

std::vector<std::string> FamiliaService::getImagenesByFamiliaId(int familia_id) {

    auto imagenes = repository->getImagenes(familia_id);
    std::vector<std::string> urls;

    for (const auto& imagen : imagenes) {
        urls.push_back(imagen.getUrl());
    }

    return urls;
}

// Implementación del método que faltaba
std::string FamiliaService::subirImagen(int familia_id, const std::vector<uint8_t>& imagen_data, bool es_principal) {
    if (imagen_data.empty()) {
        throw std::invalid_argument("Los datos de la imagen no pueden estar vacíos");
    }

    // Convertir vector<uint8_t> a string para usar con addImagenToFamilia
    std::string image_data(imagen_data.begin(), imagen_data.end());
    return addImagenToFamilia(familia_id, image_data,es_principal);
}