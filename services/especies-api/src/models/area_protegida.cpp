#include "../../include/models/area_protegida.hpp"

#include <stdexcept>

std::string areaProtegidaTipoToString(AreaProtegidaTipo tipo) {
    switch (tipo) {
        case AreaProtegidaTipo::ParqueNacional: return "parque_nacional";
        case AreaProtegidaTipo::ReservaNacional: return "reserva_nacional";
        case AreaProtegidaTipo::MonumentoNatural: return "monumento_natural";
        case AreaProtegidaTipo::SantuarioNaturaleza: return "santuario_naturaleza";
        case AreaProtegidaTipo::ParquePrivado: return "parque_privado";
        case AreaProtegidaTipo::SitioRamsar: return "sitio_ramsar";
        case AreaProtegidaTipo::HumedalUrbano: return "humedal_urbano";
    }
    throw std::invalid_argument("tipo de área protegida desconocido");
}

AreaProtegidaTipo areaProtegidaTipoFromString(const std::string& value) {
    if (value == "parque_nacional") return AreaProtegidaTipo::ParqueNacional;
    if (value == "reserva_nacional") return AreaProtegidaTipo::ReservaNacional;
    if (value == "monumento_natural") return AreaProtegidaTipo::MonumentoNatural;
    if (value == "santuario_naturaleza") return AreaProtegidaTipo::SantuarioNaturaleza;
    if (value == "parque_privado") return AreaProtegidaTipo::ParquePrivado;
    if (value == "sitio_ramsar") return AreaProtegidaTipo::SitioRamsar;
    if (value == "humedal_urbano") return AreaProtegidaTipo::HumedalUrbano;
    throw std::invalid_argument("tipo de área protegida inválido: " + value);
}

bool AreaProtegida::esValida() const {
    if (nombre.empty()) {
        return false;
    }
    if (min_lat > max_lat || min_lng > max_lng) {
        return false;
    }
    if (centro_lat < min_lat || centro_lat > max_lat ||
        centro_lng < min_lng || centro_lng > max_lng) {
        return false;
    }
    if (min_lat < -90 || max_lat > 90 || min_lng < -180 || max_lng > 180) {
        return false;
    }
    return true;
}

nlohmann::json AreaProtegida::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["nombre"] = nombre;
    json["tipo"] = areaProtegidaTipoToString(tipo);
    json["descripcion"] = descripcion ? nlohmann::json(*descripcion) : nlohmann::json(nullptr);
    json["administrador"] = administrador ? nlohmann::json(*administrador) : nlohmann::json(nullptr);
    json["accesos"] = accesos ? nlohmann::json(*accesos) : nlohmann::json(nullptr);
    json["sitio_web"] = sitio_web ? nlohmann::json(*sitio_web) : nlohmann::json(nullptr);
    json["centro_lat"] = centro_lat;
    json["centro_lng"] = centro_lng;
    json["bbox"] = {min_lng, min_lat, max_lng, max_lat};  // orden GeoJSON
    json["geometria"] = geometria ? *geometria : nlohmann::json(nullptr);
    json["superficie_ha"] = superficie_ha ? nlohmann::json(*superficie_ha) : nlohmann::json(nullptr);
    json["fuente"] = fuente ? nlohmann::json(*fuente) : nlohmann::json(nullptr);
    json["verificado"] = verificado;
    json["created_at"] = created_at ? nlohmann::json(*created_at) : nlohmann::json(nullptr);
    json["updated_at"] = updated_at ? nlohmann::json(*updated_at) : nlohmann::json(nullptr);
    return json;
}

nlohmann::json EspecieEnArea::toJson() const {
    nlohmann::json json;
    json["especie_id"] = especie_id;
    json["nombre_comun"] = nombre_comun;
    json["nombre_cientifico"] = nombre_cientifico;
    json["reino"] = reinoToString(reino);
    json["avistamientos"] = avistamientos;
    json["ultimo_avistamiento"] = ultimo_avistamiento
        ? nlohmann::json(*ultimo_avistamiento) : nlohmann::json(nullptr);
    return json;
}
