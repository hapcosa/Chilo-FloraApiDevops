#include "../../include/models/familia.hpp"

#include <algorithm>
#include <stdexcept>

Familia::Familia() : id(0), reino(Reino::Plantae) {}

bool Familia::esValida() const {
    if (nombre.empty() || nombre.length() > 150) return false;
    if (descripcion.length() > 1500) return false;
    return true;
}

std::vector<std::string> Familia::getImagenesUrls() const { return imagenes_urls; }

void Familia::setImagenesUrls(const std::vector<std::string>& v) {
    imagenes_urls = v;
}

void Familia::addImagenUrl(const std::string& imagen_url) {
    if (std::find(imagenes_urls.begin(), imagenes_urls.end(), imagen_url)
        == imagenes_urls.end()) {
        imagenes_urls.push_back(imagen_url);
    }
}

void Familia::removeImagenUrl(const std::string& imagen_url) {
    imagenes_urls.erase(
        std::remove(imagenes_urls.begin(), imagenes_urls.end(), imagen_url),
        imagenes_urls.end());
}

std::string Familia::getImagenPrincipal() const { return imagen_principal; }

void Familia::setImagenPrincipal(const std::string& imagen_url) {
    imagen_principal = imagen_url;
}

std::string Familia::getImagenUrl() const { return getImagenPrincipal(); }

void Familia::setImagenUrl(const std::string& imagen_url) {
    imagenes_urls.clear();
    if (!imagen_url.empty()) {
        imagenes_urls.push_back(imagen_url);
    }
}

nlohmann::json Familia::toJson() const {
    nlohmann::json j = {
        {"id", id},
        {"reino", reinoToString(reino)},
        {"nombre", nombre},
        {"descripcion", descripcion},
        {"imagen_principal", imagen_principal},
        {"imagenes_urls", imagenes_urls},
    };
    if (created_at.has_value()) {
        j["created_at"] = *created_at;
    } else {
        j["created_at"] = nullptr;
    }
    return j;
}

Familia Familia::fromJson(const nlohmann::json& j) {
    Familia familia;

    if (j.contains("id") && j["id"].is_number_integer()) {
        familia.setId(j["id"].get<int>());
    }

    if (!j.contains("reino") || !j["reino"].is_string()) {
        throw std::invalid_argument(
            "El campo 'reino' es obligatorio (uno de: animalia, plantae, fungi, "
            "protista, monera)");
    }
    familia.setReino(reinoFromString(j["reino"].get<std::string>()));

    if (!j.contains("nombre") || !j["nombre"].is_string()
        || j["nombre"].get<std::string>().empty()) {
        throw std::invalid_argument("El nombre de la familia es obligatorio");
    }
    familia.setNombre(j["nombre"].get<std::string>());
    familia.setDescripcion(j.value("descripcion", ""));

    if (j.contains("imagenes_urls") && j["imagenes_urls"].is_array()) {
        familia.setImagenesUrls(j["imagenes_urls"].get<std::vector<std::string>>());
    } else if (j.contains("imagen_url") && j["imagen_url"].is_string()) {
        familia.setImagenUrl(j["imagen_url"].get<std::string>());
    }

    if (!familia.esValida()) {
        throw std::invalid_argument("Los datos de la familia no son válidos");
    }
    return familia;
}
