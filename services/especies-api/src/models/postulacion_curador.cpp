#include "../../include/models/postulacion_curador.hpp"

#include <cctype>
#include <stdexcept>

namespace {

// El texto es la evidencia que el admin lee para decidir; el tope evita que
// una postulación se use como almacén de texto arbitrario.
constexpr size_t kTextoMaxLength = 4000;

template <typename T>
void writeOpt(nlohmann::json& json, const std::string& key, const std::optional<T>& value) {
    if (value) {
        json[key] = *value;
    } else {
        json[key] = nullptr;
    }
}

bool esBlanco(const std::string& value) {
    for (const char c : value) {
        if (!std::isspace(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

} // namespace

std::string postulacionEstadoToString(PostulacionEstado estado) {
    switch (estado) {
        case PostulacionEstado::Pendiente: return "pendiente";
        case PostulacionEstado::Aprobada:  return "aprobada";
        case PostulacionEstado::Rechazada: return "rechazada";
    }
    throw std::invalid_argument("estado de postulación desconocido");
}

PostulacionEstado postulacionEstadoFromString(const std::string& value) {
    if (value == "pendiente") return PostulacionEstado::Pendiente;
    if (value == "aprobada")  return PostulacionEstado::Aprobada;
    if (value == "rechazada") return PostulacionEstado::Rechazada;
    throw std::invalid_argument(
        "'estado' debe ser uno de: pendiente, aprobada, rechazada");
}

bool PostulacionCurador::esValida() const {
    if (usuario_id <= 0 || categoria_id <= 0) return false;
    if (texto.empty() || esBlanco(texto) || texto.length() > kTextoMaxLength) return false;
    if (estado == PostulacionEstado::Rechazada && !motivo) return false;
    return true;
}

nlohmann::json PostulacionCurador::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["usuario_id"] = usuario_id;
    json["categoria_id"] = categoria_id;
    json["texto"] = texto;
    json["estado"] = postulacionEstadoToString(estado);
    writeOpt(json, "revisado_por", revisado_por);
    writeOpt(json, "revisado_en", revisado_en);
    writeOpt(json, "motivo", motivo);
    writeOpt(json, "created_at", created_at);
    writeOpt(json, "updated_at", updated_at);
    return json;
}

PostulacionCurador PostulacionCurador::fromJson(const nlohmann::json& json) {
    PostulacionCurador postulacion;

    if (!json.contains("categoria_id") || !json["categoria_id"].is_number_integer()) {
        throw std::invalid_argument("'categoria_id' es obligatorio y debe ser entero");
    }
    postulacion.categoria_id = json["categoria_id"].get<int>();

    if (!json.contains("texto") || !json["texto"].is_string()) {
        throw std::invalid_argument("'texto' es obligatorio");
    }
    postulacion.texto = json["texto"].get<std::string>();

    if (esBlanco(postulacion.texto)) {
        throw std::invalid_argument("'texto' no puede estar en blanco");
    }
    if (postulacion.texto.length() > kTextoMaxLength) {
        throw std::invalid_argument("'texto' supera los 4000 caracteres");
    }

    return postulacion;
}
