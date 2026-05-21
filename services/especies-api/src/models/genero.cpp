#include "../../include/models/genero.hpp"

#include <algorithm>
#include <stdexcept>

Genero::Genero() : id(0), familia_id(0) {}

bool Genero::esValida() const {
    if (nombre.empty() || nombre.length() > 150) return false;
    if (descripcion.length() > 1500) return false;
    if (familia_id <= 0) return false;
    return true;
}

std::vector<std::string> Genero::getImagenesUrls() const { return imagenes_urls; }
void Genero::setImagenesUrls(const std::vector<std::string>& v) { imagenes_urls = v; }

void Genero::addImagenUrl(const std::string& imagen_url) {
    if (std::find(imagenes_urls.begin(), imagenes_urls.end(), imagen_url)
        == imagenes_urls.end()) {
        imagenes_urls.push_back(imagen_url);
    }
}

void Genero::removeImagenUrl(const std::string& imagen_url) {
    imagenes_urls.erase(
        std::remove(imagenes_urls.begin(), imagenes_urls.end(), imagen_url),
        imagenes_urls.end());
}

std::string Genero::getImagenPrincipal() const { return imagen_principal; }

void Genero::setImagenPrincipal(const std::string& imagen_url) {
    imagen_principal = imagen_url;
}

std::string Genero::getImagenUrl() const { return getImagenPrincipal(); }

void Genero::setImagenUrl(const std::string& imagen_url) {
    imagenes_urls.clear();
    if (!imagen_url.empty()) {
        imagenes_urls.push_back(imagen_url);
    }
}

nlohmann::json Genero::toJson() const {
    nlohmann::json j = {
        {"id", id},
        {"nombre", nombre},
        {"descripcion", descripcion},
        {"familia_id", familia_id},
    };
    if (created_at.has_value()) {
        j["created_at"] = *created_at;
    } else {
        j["created_at"] = nullptr;
    }
    if (familia_nombre.has_value()) {
        j["familia_nombre"] = *familia_nombre;
    }
    return j;
}

Genero Genero::fromJson(const nlohmann::json& j) {
    Genero g;

    if (j.contains("id") && j["id"].is_number_integer()) {
        g.setId(j["id"].get<int>());
    }

    if (!j.contains("nombre") || !j["nombre"].is_string()
        || j["nombre"].get<std::string>().empty()) {
        throw std::invalid_argument("El nombre del género es obligatorio");
    }
    g.setNombre(j["nombre"].get<std::string>());
    g.setDescripcion(j.value("descripcion", ""));

    if (!j.contains("familia_id") || !j["familia_id"].is_number_integer()
        || j["familia_id"].get<int>() <= 0) {
        throw std::invalid_argument("'familia_id' es obligatorio y > 0");
    }
    g.setFamiliaId(j["familia_id"].get<int>());

    if (j.contains("imagenes_urls") && j["imagenes_urls"].is_array()) {
        g.setImagenesUrls(j["imagenes_urls"].get<std::vector<std::string>>());
    } else if (j.contains("imagen_url") && j["imagen_url"].is_string()) {
        g.setImagenUrl(j["imagen_url"].get<std::string>());
    }

    if (!g.esValida()) {
        throw std::invalid_argument("Los datos del género no son válidos");
    }
    return g;
}
