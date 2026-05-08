#include "../../include/models/especie.hpp"
#include <stdexcept>
#include <algorithm>

Especie::Especie() : id(0), genero_id(0), endemica(false) {}

Especie::Especie(int id, const std::string& nombre_cientifico, const std::string& nombre_comun,
                 int genero_id, const std::string& descripcion, const std::string& habitat,
                 const std::string& distribucion, bool endemica, const std::string& estado_conservacion)
        : id(id), nombre_cientifico(nombre_cientifico), nombre_comun(nombre_comun),
          genero_id(genero_id), descripcion(descripcion), habitat(habitat),
          distribucion(distribucion), endemica(endemica), estado_conservacion(estado_conservacion) {}

// Getters y setters
int Especie::getId() const { return id; }
void Especie::setId(int id) { this->id = id; }

std::string Especie::getNombreCientifico() const { return nombre_cientifico; }
void Especie::setNombreCientifico(const std::string& nombre_cientifico) {
    this->nombre_cientifico = nombre_cientifico;
}

std::string Especie::getNombreComun() const { return nombre_comun; }
void Especie::setNombreComun(const std::string& nombre_comun) {
    this->nombre_comun = nombre_comun;
}

int Especie::getGeneroId() const { return genero_id; }
void Especie::setGeneroId(int genero_id) {
    this->genero_id = genero_id;
}

std::string Especie::getDescripcion() const { return descripcion; }
void Especie::setDescripcion(const std::string& descripcion) {
    this->descripcion = descripcion;
}

// Métodos para manejar múltiples imágenes
std::vector<std::string> Especie::getImagenesUrls() const {
    return imagenes_urls;
}

void Especie::setImagenesUrls(const std::vector<std::string>& imagenes_urls) {
    this->imagenes_urls = imagenes_urls;
}

void Especie::addImagenUrl(const std::string& imagen_url) {
    // Evitar duplicados
    if (std::find(imagenes_urls.begin(), imagenes_urls.end(), imagen_url) == imagenes_urls.end()) {
        imagenes_urls.push_back(imagen_url);
    }
}

void Especie::removeImagenUrl(const std::string& imagen_url) {
    imagenes_urls.erase(
            std::remove(imagenes_urls.begin(), imagenes_urls.end(), imagen_url),
            imagenes_urls.end()
    );
}

std::string Especie::getImagenPrincipal() const {
    return imagenes_urls.empty() ? "" : imagenes_urls[0];
}

std::string Especie::getHabitat() const { return habitat; }
void Especie::setHabitat(const std::string& habitat) {
    this->habitat = habitat;
}

std::string Especie::getDistribucion() const { return distribucion; }
void Especie::setDistribucion(const std::string& distribucion) {
    this->distribucion = distribucion;
}

bool Especie::isEndemica() const { return endemica; }
void Especie::setEndemica(bool endemica) {
    this->endemica = endemica;
}

std::string Especie::getEstadoConservacion() const { return estado_conservacion; }
void Especie::setEstadoConservacion(const std::string& estado_conservacion) {
    this->estado_conservacion = estado_conservacion;
}

std::optional<std::string> Especie::getGeneroNombre() const { return genero_nombre; }
void Especie::setGeneroNombre(const std::string& genero_nombre) {
    this->genero_nombre = genero_nombre;
}

// Validación
bool Especie::esValida() const {
    if (nombre_cientifico.empty() || nombre_cientifico.length() > 100) {
        return false;
    }

    if (nombre_comun.length() > 100) return false;
    if (estado_conservacion.length() > 50) return false;
    if (descripcion.length() > 1500) return false;
    return true;
}

// Serialización a JSON
nlohmann::json Especie::toJson() const {
    nlohmann::json j = {
            {"id", id},
            {"nombre_cientifico", nombre_cientifico},
            {"nombre_comun", nombre_comun},
            {"genero_id", genero_id},
            {"descripcion", descripcion},
            {"habitat", habitat},
            {"distribucion", distribucion},
            {"endemica", endemica},
            {"estado_conservacion", estado_conservacion},
            {"imagenes_urls", imagenes_urls}
    };

    if (genero_nombre) {
        j["genero_nombre"] = *genero_nombre;
    }

    return j;
}

Especie Especie::fromJson(const nlohmann::json& j) {
    Especie especie;

    if (j.contains("id")) {
        especie.id = j["id"].get<int>();
    }

    if (!j.contains("nombre_cientifico") || j["nombre_cientifico"].empty()) {
        throw std::invalid_argument("El nombre científico es obligatorio");
    }

    especie.nombre_cientifico = j["nombre_cientifico"].get<std::string>();
    especie.nombre_comun = j.value("nombre_comun", "");
    especie.genero_id = j.value("genero_id", 0);
    especie.descripcion = j.value("descripcion", "");
    especie.habitat = j.value("habitat", "");
    especie.distribucion = j.value("distribucion", "");
    especie.endemica = j.value("endemica", false);
    especie.estado_conservacion = j.value("estado_conservacion", "");

    // Cargar imágenes
    if (j.contains("imagenes_urls") && j["imagenes_urls"].is_array()) {
        especie.imagenes_urls = j["imagenes_urls"].get<std::vector<std::string>>();
    } else if (j.contains("imagen_url")) {
        // Compatibilidad con formato anterior
        std::string imagen = j["imagen_url"].get<std::string>();
        if (!imagen.empty()) {
            especie.imagenes_urls.push_back(imagen);
        }
    }

    if (j.contains("genero_nombre")) {
        especie.genero_nombre = j["genero_nombre"].get<std::string>();
    }

    if (!especie.esValida()) {
        throw std::invalid_argument("Los datos de la especie no son válidos");
    }

    return especie;
}