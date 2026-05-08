#include "../../include/models/familia.hpp"

#include <algorithm>  // Para std::find y std::remove
#include <stdexcept>

// Validación
bool Familia::esValida() const {
  // El nombre es obligatorio
  if (nombre.empty() || nombre.length() > 150) {
    return false;
  }

  // Validación de descripción
  if (descripcion.empty() || descripcion.length() > 1500) {
    return false;
  }

  return true;
}

// Métodos para manejar múltiples imágenes
std::vector<std::string> Familia::getImagenesUrls() const {
  return imagenes_urls;
}

void Familia::setImagenesUrls(const std::vector<std::string>& imagenes_urls) {
  this->imagenes_urls = imagenes_urls;
}

void Familia::addImagenUrl(const std::string& imagen_url) {
  // Evitar duplicados
  if (std::find(imagenes_urls.begin(), imagenes_urls.end(), imagen_url) ==
      imagenes_urls.end()) {
    imagenes_urls.push_back(imagen_url);
  }
}

void Familia::removeImagenUrl(const std::string& imagen_url) {
  imagenes_urls.erase(
      std::remove(imagenes_urls.begin(), imagenes_urls.end(), imagen_url),
      imagenes_urls.end());
}

std::string Familia::getImagenPrincipal() const {
  return imagen_principal.empty() ? "" : imagen_principal;
}

// Métodos de compatibilidad con imagen única
std::string Familia::getImagenUrl() const {
  return getImagenPrincipal();  // Devuelve la primera imagen
}

void Familia::setImagenUrl(const std::string& imagen_url) {
  imagenes_urls.clear();  // Limpia las imágenes existentes
  if (!imagen_url.empty()) {
    imagenes_urls.push_back(imagen_url);
  }
}
void Familia::setImagenPrincipal(const std::string& imagen_url) {
  imagen_principal = imagen_url;
}
// Serialización a JSON
nlohmann::json Familia::toJson() const {
  nlohmann::json j = {{"id", id},
                      {"nombre", nombre},
                      {"descripcion", descripcion},
                      {"imagen_principal", imagen_principal},
                      {"imagenes_urls", imagenes_urls}};

  // Incluir imágenes si existen

  return j;
}

// Deserialización desde JSON
Familia Familia::fromJson(const nlohmann::json& j) {
  Familia familia;
  // ID es opcional cuando se crea una nueva familia
  if (j.contains("id") && !j["id"].is_null()) {
    familia.id = j["id"].get<int>();
  } else {
    familia.id = 0;  // Valor por defecto
  }

  // Validación de campos obligatorios
  if (!j.contains("nombre") || j["nombre"].is_null() ||
      j["nombre"].get<std::string>().empty()) {
    throw std::invalid_argument("El nombre de la familia es obligatorio");
  }

  familia.nombre = j["nombre"].get<std::string>();

  // Descripción es opcional
  familia.descripcion = j.value("descripcion", "");

  // CORREGIDO: Manejo de imágenes
  if (j.contains("imagenes_urls") && j["imagenes_urls"].is_array()) {
    // Si existe el campo imagenes_urls como array
    familia.imagenes_urls = j["imagenes_urls"].get<std::vector<std::string>>();
  } else if (j.contains("imagen_url") && !j["imagen_url"].empty()) {
    // Compatibilidad con formato anterior (imagen_url único)
    std::string imagen = j["imagen_url"].get<std::string>();
    if (!imagen.empty()) {
      familia.imagenes_urls.push_back(imagen);
    }
  }

  // Validación final
  if (!familia.esValida()) {
    throw std::invalid_argument("Los datos de la familia no son válidos");
  }

  return familia;
}
