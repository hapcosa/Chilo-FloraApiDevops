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
  return json[key].get<std::string>();
}

}  // namespace

bool CategoriaModeracion::esValida() const { return !nombre.empty(); }

nlohmann::json CategoriaModeracion::toJson() const {
  nlohmann::json j;
  j["id"] = id;
  j["reino"] = reinoToString(reino);
  j["nombre"] = nombre;
  writeOpt(j, "descripcion", descripcion);
  writeOpt(j, "created_at", created_at);
  return j;
}

CategoriaModeracion CategoriaModeracion::fromJson(const nlohmann::json& json) {
  CategoriaModeracion categoria;

  if (json.contains("id") && json["id"].is_number_integer()) {
    categoria.setId(json["id"].get<int>());
  }

  if (!json.contains("reino") || !json["reino"].is_string()) {
    throw std::invalid_argument("'reino' es obligatorio");
  }
  categoria.setReino(reinoFromString(json["reino"].get<std::string>()));

  if (!json.contains("nombre") || !json["nombre"].is_string() ||
      json["nombre"].get<std::string>().empty()) {
    throw std::invalid_argument("'nombre' es obligatorio");
  }
  categoria.setNombre(json["nombre"].get<std::string>());

  categoria.setDescripcion(optString(json, "descripcion"));

  return categoria;
}
