#include "../../include/services/genero_service.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

GeneroService::GeneroService(std::shared_ptr<IGeneroRepository> repo)
    : repository(repo) {}

void GeneroService::validateGenero(const Genero& genero) {
  if (genero.getNombre().empty()) {
    throw std::invalid_argument("El nombre del género no puede estar vacío");
  }
  if (genero.getFamiliaId() <= 0) {
    throw std::invalid_argument("'familia_id' debe ser un entero > 0");
  }
}

std::vector<Genero> GeneroService::getAllGeneros() {
  return repository->getAll();
}

std::optional<Genero> GeneroService::getGeneroById(int id) {
  if (id <= 0) {
    throw std::invalid_argument("ID debe ser mayor que 0");
  }
  return repository->findById(id);
}

std::optional<Genero> GeneroService::findByNombre(int familia_id,
                                                   const std::string& nombre) {
  if (familia_id <= 0) {
    throw std::invalid_argument("'familia_id' debe ser > 0");
  }
  if (nombre.empty()) {
    throw std::invalid_argument("nombre no puede estar vacío");
  }
  return repository->findByNombre(familia_id, nombre);
}

std::vector<Genero> GeneroService::searchByFamilia(const std::string& familia) {
  if (familia.empty()) {
    throw std::invalid_argument("nombre de familia no puede estar vacío");
  }
  return repository->getByFamilia(familia);
}

Genero GeneroService::createGenero(const Genero& genero) {
  validateGenero(genero);
  if (repository->findByNombre(genero.getFamiliaId(), genero.getNombre())) {
    throw std::invalid_argument(
        "Ya existe un género con ese nombre en la misma familia");
  }
  return repository->create(genero);
}

Genero GeneroService::updateGenero(const Genero& genero) {
  if (genero.getId() <= 0) {
    throw std::invalid_argument("ID debe ser mayor que 0");
  }

  if (!repository->findById(genero.getId())) {
    throw std::runtime_error("Género no encontrado");
  }
  validateGenero(genero);
  auto existing = repository->findByNombre(genero.getFamiliaId(),
                                            genero.getNombre());
  if (existing && existing->getId() != genero.getId()) {
    throw std::invalid_argument(
        "Ya existe otro género con ese nombre en la misma familia");
  }
  return repository->update(genero);
}

bool GeneroService::deleteGenero(int id) {
  if (id <= 0) {
    throw std::invalid_argument("ID debe ser mayor que 0");
  }

  if (!repository->findById(id)) {
    throw std::runtime_error("Genero no encontrado");
  }
  return repository->remove(id);
}

std::string GeneroService::addImagenToGenero(int genero_id,
                                             const std::string& image_data,
                                             bool es_principal) {
  if (genero_id <= 0) {
    throw std::invalid_argument("ID de genero debe ser mayor que 0");
  }

  if (image_data.empty()) {
    throw std::invalid_argument(
        "Los datos de la imagen no pueden estar vacíos");
  }

  if (!repository->findById(genero_id)) {
    throw std::runtime_error("Familia no encontrada");
  }

  std::string dir_path = "./static/images/generos/";
  if (!std::filesystem::exists(dir_path)) {
    std::filesystem::create_directories(dir_path);
  }

  // 3. Generar nombre único y guardar imagen
  auto now = std::chrono::system_clock::now();
  auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch())
                       .count();
  std::string filename = "genero_" + std::to_string(genero_id) + "_" +
                         std::to_string(timestamp) + ".jpg";
  std::string image_path = dir_path + filename;

  try {
    std::ofstream file(image_path, std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("No se pudo crear el archivo de imagen en: " +
                               image_path);
    }
    file.write(image_data.data(), image_data.size());
    file.close();

    // Verificar que el archivo se creó correctamente
    if (!std::filesystem::exists(image_path)) {
      throw std::runtime_error("El archivo no se guardó correctamente en: " +
                               image_path);
    }

    // 4. Guardar referencia en la base de datos
    std::string public_url = "/api/images/generos/" + filename;
    if (!repository->agregarImagen(genero_id, public_url, es_principal)) {
      std::filesystem::remove(image_path);  // Revertir si falla la BD
      throw std::runtime_error(
          "Error al guardar imagen en la base de datos para genero: " +
          std::to_string(genero_id));
    }

    std::cout << "Imagen guardada exitosamente en: " << image_path << std::endl;
    return public_url;
  } catch (const std::exception& e) {
    std::cout << "Error al guardar imagen: " << e.what() << std::endl;
    std::filesystem::remove(image_path);  // Limpiar en caso de error
    throw;
  }
}

// También corrige el método removeImagenFromGenero
bool GeneroService::removeImagenFromGenero(int genero_id,
                                           const std::string& image_url) {
  if (genero_id <= 0) {
    throw std::invalid_argument("ID de genero debe ser mayor que 0");
  }

  if (image_url.empty()) {
    throw std::invalid_argument("URL de imagen no puede estar vacía");
  }

  // 1. Extraer el nombre del archivo de la URL
  std::string filename = image_url.substr(image_url.find_last_of('/') + 1);
  std::string filepath =
      "./static/images/generos/" + filename;  // RUTA CORREGIDA

  // 2. Eliminar de la base de datos primero
  if (repository->eliminarImagen(genero_id, image_url)) {
    // 3. Si éxito en BD, eliminar el archivo
    std::filesystem::remove(filepath);
    return true;
  }
  return false;
}
bool GeneroService::setImagenPrincipal(int genero_id,
                                       const std::string& image_url) {
  if (genero_id <= 0) {
    throw std::invalid_argument("ID de genero debe ser mayor que 0");
  }

  if (image_url.empty()) {
    throw std::invalid_argument("URL de imagen no puede estar vacía");
  }

  // Verificar que la genero existe
  if (!repository->findById(genero_id)) {
    throw std::runtime_error("Genero no encontrada");
  }

  return repository->setImagenPrincipal(genero_id, image_url);
}

std::vector<std::string> GeneroService::getImagenesByGeneroId(int genero_id) {
  auto imagenes = repository->getImagenes(genero_id);
  std::vector<std::string> urls;

  for (const auto& imagen : imagenes) {
    urls.push_back(imagen.getUrl());
  }

  return urls;
}

std::string GeneroService::subirImagen(int genero_id,
                                       const std::vector<uint8_t>& imagen_data,
                                       bool es_principal) {
  if (imagen_data.empty()) {
    throw std::invalid_argument(
        "Los datos de la imagen no pueden estar vacíos");
  }

  // Convertir vector<uint8_t> a string para usar con addImagenToGenero
  std::string image_data(imagen_data.begin(), imagen_data.end());
  return addImagenToGenero(genero_id, image_data, es_principal);
}
