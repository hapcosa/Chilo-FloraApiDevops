#include "../../include/models/genero.hpp"

#include <stdexcept>

// Validación
bool Genero::esValida() const {
  // El nombre es obligatorio
  if (nombre.empty() || nombre.length() > 100) {
    return false;
  }

  // Otras validaciones según sea necesario
  if (descripcion.length() > 1500) {
    return false;
  }

  return true;
}
// Metodos imagenes
std::vector<std::string> Genero::getImagenesUrls() const {
  return imagenes_urls;
}
void Genero::setImagenesUrls(const std::vector<std::string>& imagenes_urls) {
  this->imagenes_urls = imagenes_urls;
}
void Genero::addImagenUrl(const std::string& imagen_url) {
  if (std::find(imagenes_urls.begin(), imagenes_urls.end(), imagen_url) ==
      imagenes_urls.end()) {
    imagenes_urls.push_back(imagen_url);
  }
}
void Genero::removeImagenUrl(const std::string& imagen_url) {
  imagenes_urls.erase(
      std::remove(imagenes_urls.begin(), imagenes_urls.end(), imagen_url),
      imagenes_urls.end());
}
std::string Genero::getImagenPrincipal() const {
  return imagen_principal.empty() ? "" : imagen_principal;
}

// Métodos de compatibilidad con imagen única
std::string Genero::getImagenUrl() const {
  return getImagenPrincipal();  // Devuelve la primera imagen
}

void Genero::setImagenUrl(const std::string& imagen_url) {
  imagenes_urls.clear();  // Limpia las imágenes existentes
  if (!imagen_url.empty()) {
    imagenes_urls.push_back(imagen_url);
  }
}
void Genero::setImagenPrincipal(const std::string& imagen_url) {
  imagen_principal = imagen_url;
}
// Serialización a JSON
nlohmann::json Genero::toJson() const {
  nlohmann::json j = {{"id", id},
                      {"nombre", nombre},
                      {"descripcion", descripcion},
                      {"familia_id", familia_id}};
  return j;
}

Genero Genero::fromJson(const nlohmann::json& j) {
  Genero genero;

  // ID es opcional cuando se crea una nueva genero
  if (j.contains("id")) {
    genero.id = j["id"].get<int>();
  }

  // Validación de campos obligatorios
  if (!j.contains("nombre") || j["nombre"].empty()) {
    throw std::invalid_argument("El nombre de la genero es obligatorio");
  }

  genero.nombre = j["nombre"].get<std::string>();
  genero.descripcion = j.value("descripcion", "");
  genero.familia_id = j.value("familia_id", 0);
  if (j.contains("imagenes_urls") && j["imagenes_urls"].is_array()) {
    // Si existe el campo imagenes_urls como array
    genero.imagenes_urls = j["imagenes_urls"].get<std::vector<std::string>>();
  } else if (j.contains("imagen_url") && !j["imagen_url"].empty()) {
    // Compatibilidad con formato anterior (imagen_url único)
    std::string imagen = j["imagen_url"].get<std::string>();
    if (!imagen.empty()) {
      genero.imagenes_urls.push_back(imagen);
    }
  }
  // Validación de datos
  if (!genero.esValida()) {
    throw std::invalid_argument("Los datos de la genero no son válidos");
  }

  return genero;
}
