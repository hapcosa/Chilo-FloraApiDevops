#include "../../include/models/categoria_moderacion.hpp"

#include <stdexcept>

namespace {

template <typename T>
void writeOpt(nlohmann::json& json, const std::string& key, const std::optional<T>& value) {
    if (value) {
        json[key] = *value;
    } else {
        json[key] = nullptr;
    }
}

std::optional<std::string> optString(const nlohmann::json& json, const std::string& key) {
    if (!json.contains(key) || json[key].is_null()) {
        return std::nullopt;
    }
    if (!json[key].is_string()) {
        throw std::invalid_argument("'" + key + "' debe ser string");
    }
    const auto value = json[key].get<std::string>();
    return value.empty() ? std::nullopt : std::optional<std::string>(value);
}

} // namespace

bool esSlugValido(const std::string& slug) {
    if (slug.empty() || slug.length() > 60) return false;
    if (slug.front() == '-' || slug.back() == '-') return false;

    bool previousWasHyphen = false;
    for (const char c : slug) {
        const bool isLowerAlnum =
            (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        if (isLowerAlnum) {
            previousWasHyphen = false;
            continue;
        }
        if (c != '-' || previousWasHyphen) return false;
        previousWasHyphen = true;
    }
    return true;
}

bool CategoriaModeracion::esValida() const {
    if (!esSlugValido(slug)) return false;
    if (nombre.empty() || nombre.length() > 120) return false;
    return true;
}

nlohmann::json CategoriaModeracion::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["slug"] = slug;
    json["nombre"] = nombre;
    json["reino"] = reinoToString(reino);
    writeOpt(json, "descripcion", descripcion);
    writeOpt(json, "created_at", created_at);
    writeOpt(json, "updated_at", updated_at);
    return json;
}

CategoriaModeracion CategoriaModeracion::fromJson(const nlohmann::json& json) {
    CategoriaModeracion categoria;

    if (json.contains("id") && json["id"].is_number_integer()) {
        categoria.id = json["id"].get<int>();
    }

    if (!json.contains("slug") || !json["slug"].is_string()) {
        throw std::invalid_argument("'slug' es obligatorio");
    }
    categoria.slug = json["slug"].get<std::string>();

    if (!json.contains("nombre") || !json["nombre"].is_string()) {
        throw std::invalid_argument("'nombre' es obligatorio");
    }
    categoria.nombre = json["nombre"].get<std::string>();

    if (!json.contains("reino") || !json["reino"].is_string()) {
        throw std::invalid_argument("'reino' es obligatorio");
    }
    categoria.reino = reinoFromString(json["reino"].get<std::string>());

    categoria.descripcion = optString(json, "descripcion");

    return categoria;
}
