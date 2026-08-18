#include "../../include/models/avistamiento.hpp"

#include <algorithm>
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

std::optional<int> optInt(const nlohmann::json& json, const std::string& key) {
    if (!json.contains(key) || json[key].is_null()) {
        return std::nullopt;
    }
    if (!json[key].is_number_integer()) {
        throw std::invalid_argument("'" + key + "' debe ser entero");
    }
    return json[key].get<int>();
}

std::optional<double> optDouble(const nlohmann::json& json, const std::string& key) {
    if (!json.contains(key) || json[key].is_null()) {
        return std::nullopt;
    }
    if (!json[key].is_number()) {
        throw std::invalid_argument("'" + key + "' debe ser numérico");
    }
    return json[key].get<double>();
}

} // namespace

std::string avistamientoEstadoToString(AvistamientoEstado estado) {
    switch (estado) {
        case AvistamientoEstado::Pendiente:
            return "pendiente";
        case AvistamientoEstado::Aprobado:
            return "aprobado";
        case AvistamientoEstado::Rechazado:
            return "rechazado";
    }
    return "pendiente";
}

AvistamientoEstado avistamientoEstadoFromString(const std::string& value) {
    if (value == "pendiente") return AvistamientoEstado::Pendiente;
    if (value == "aprobado") return AvistamientoEstado::Aprobado;
    if (value == "rechazado") return AvistamientoEstado::Rechazado;
    throw std::invalid_argument("estado de avistamiento inválido: " + value);
}

std::string avistamientoVisibilidadToString(AvistamientoVisibilidad visibilidad) {
    switch (visibilidad) {
        case AvistamientoVisibilidad::Privado:
            return "privado";
        case AvistamientoVisibilidad::Publico:
            return "publico";
    }
    return "privado";
}

AvistamientoVisibilidad avistamientoVisibilidadFromString(const std::string& value) {
    if (value == "privado") return AvistamientoVisibilidad::Privado;
    if (value == "publico") return AvistamientoVisibilidad::Publico;
    throw std::invalid_argument("visibilidad de avistamiento inválida: " + value);
}

std::string precisionDeclaradaToString(PrecisionDeclarada precision) {
    switch (precision) {
        case PrecisionDeclarada::Exacto:
            return "exacto";
        case PrecisionDeclarada::Aproximado:
            return "aproximado";
        case PrecisionDeclarada::Zona:
            return "zona";
    }
    return "exacto";
}

PrecisionDeclarada precisionDeclaradaFromString(const std::string& value) {
    if (value == "exacto") return PrecisionDeclarada::Exacto;
    if (value == "aproximado") return PrecisionDeclarada::Aproximado;
    if (value == "zona") return PrecisionDeclarada::Zona;
    throw std::invalid_argument("precisión declarada inválida: " + value);
}

bool Avistamiento::esValido() const {
    if (foto_key.empty() || foto_key.length() > 500) return false;
    if (geo_lat < -90 || geo_lat > 90) return false;
    if (geo_lng < -180 || geo_lng > 180) return false;
    if (precision_metros && *precision_metros < 0) return false;
    if (nombre_sugerido && nombre_sugerido->length() > 200) return false;
    if (estado == AvistamientoEstado::Rechazado && !motivo_rechazo) return false;
    return true;
}

nlohmann::json Avistamiento::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    writeOpt(json, "especie_id", especie_id);
    json["reino"] = reinoToString(reino);
    writeOpt(json, "nombre_sugerido", nombre_sugerido);
    writeOpt(json, "descripcion", descripcion);
    json["foto_key"] = foto_key;
    writeOpt(json, "foto_url", foto_url);
    json["geo_lat"] = geo_lat;
    json["geo_lng"] = geo_lng;
    writeOpt(json, "precision_metros", precision_metros);
    json["precision_declarada"] = precisionDeclaradaToString(precision_declarada);
    writeOpt(json, "observado_en", observado_en);
    writeOpt(json, "creado_por", creado_por);
    json["estado"] = avistamientoEstadoToString(estado);
    json["visibilidad"] = avistamientoVisibilidadToString(visibilidad);
    json["grado_identificacion"] = gradoIdentificacionToString(grado_identificacion);
    json["identificaciones_count"] = identificaciones_count;
    writeOpt(json, "moderado_por", moderado_por);
    writeOpt(json, "moderado_en", moderado_en);
    writeOpt(json, "motivo_rechazo", motivo_rechazo);
    writeOpt(json, "created_at", created_at);
    writeOpt(json, "updated_at", updated_at);
    return json;
}

Avistamiento Avistamiento::fromJson(const nlohmann::json& json) {
    Avistamiento avistamiento;

    if (json.contains("id") && json["id"].is_number_integer()) {
        avistamiento.id = json["id"].get<int>();
    }

    if (!json.contains("reino") || !json["reino"].is_string()) {
        throw std::invalid_argument("'reino' es obligatorio");
    }
    avistamiento.reino = reinoFromString(json["reino"].get<std::string>());

    if (!json.contains("foto_key") || !json["foto_key"].is_string() ||
        json["foto_key"].get<std::string>().empty()) {
        throw std::invalid_argument("'foto_key' es obligatorio");
    }
    avistamiento.foto_key = json["foto_key"].get<std::string>();

    if (!json.contains("geo_lat") || !json["geo_lat"].is_number()) {
        throw std::invalid_argument("'geo_lat' es obligatorio");
    }
    if (!json.contains("geo_lng") || !json["geo_lng"].is_number()) {
        throw std::invalid_argument("'geo_lng' es obligatorio");
    }
    avistamiento.geo_lat = json["geo_lat"].get<double>();
    avistamiento.geo_lng = json["geo_lng"].get<double>();

    avistamiento.especie_id = optInt(json, "especie_id");
    avistamiento.nombre_sugerido = optString(json, "nombre_sugerido");
    avistamiento.descripcion = optString(json, "descripcion");
    avistamiento.precision_metros = optDouble(json, "precision_metros");
    // A diferencia de `visibilidad`, esta sí la declara quien registra: es una
    // afirmación sobre su propio dato, no un permiso.
    if (json.contains("precision_declarada") && !json["precision_declarada"].is_null()) {
        if (!json["precision_declarada"].is_string()) {
            throw std::invalid_argument("'precision_declarada' debe ser string");
        }
        avistamiento.precision_declarada =
            precisionDeclaradaFromString(json["precision_declarada"].get<std::string>());
    }
    avistamiento.observado_en = optString(json, "observado_en");
    avistamiento.creado_por = optInt(json, "creado_por");
    avistamiento.moderado_por = optInt(json, "moderado_por");
    avistamiento.moderado_en = optString(json, "moderado_en");
    avistamiento.motivo_rechazo = optString(json, "motivo_rechazo");

    if (json.contains("estado") && json["estado"].is_string()) {
        avistamiento.estado = avistamientoEstadoFromString(json["estado"].get<std::string>());
    }

    // `visibilidad` no se lee del cuerpo a propósito: un avistamiento nace
    // privado y solo PATCH .../compartir lo publica.

    return avistamiento;
}

