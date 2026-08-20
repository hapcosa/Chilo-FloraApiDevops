#include "models/portada.hpp"

namespace {

// nlohmann serializa un optional vacío como `null` solo si se le pide; aquí
// lo hacemos explícito para que el cliente reciba siempre la clave y no tenga
// que distinguir "ausente" de "vacío".
void ponerOpcional(nlohmann::json& json,
                   const std::string& clave,
                   const std::optional<std::string>& valor) {
    if (valor.has_value()) {
        json[clave] = *valor;
    } else {
        json[clave] = nullptr;
    }
}

void ponerOpcional(nlohmann::json& json, const std::string& clave, const std::optional<int>& valor) {
    if (valor.has_value()) {
        json[clave] = *valor;
    } else {
        json[clave] = nullptr;
    }
}

}  // namespace

nlohmann::json PortadaEspecie::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["reino"] = reinoToString(reino);
    json["nombre_comun"] = nombre_comun;
    json["nombre_cientifico"] = nombre_cientifico;
    ponerOpcional(json, "foto_portada_key", foto_portada_key);
    ponerOpcional(json, "foto_url", foto_url);
    ponerOpcional(json, "fecha", fecha);
    return json;
}

nlohmann::json PortadaEncuentro::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    ponerOpcional(json, "especie_id", especie_id);
    json["reino"] = reinoToString(reino);
    ponerOpcional(json, "nombre_sugerido", nombre_sugerido);
    json["foto_key"] = foto_key;
    ponerOpcional(json, "foto_url", foto_url);
    ponerOpcional(json, "creado_por", creado_por);
    ponerOpcional(json, "observado_en", observado_en);
    ponerOpcional(json, "created_at", created_at);
    // Sin geo_lat/geo_lng: ver el comentario del struct en portada.hpp.
    return json;
}

nlohmann::json Portada::toJson() const {
    nlohmann::json json;

    json["ultimas_publicadas"] = nlohmann::json::array();
    for (const auto& especie : ultimas_publicadas) {
        json["ultimas_publicadas"].push_back(especie.toJson());
    }

    json["ultimas_ediciones"] = nlohmann::json::array();
    for (const auto& especie : ultimas_ediciones) {
        json["ultimas_ediciones"].push_back(especie.toJson());
    }

    json["ultimos_encuentros"] = nlohmann::json::array();
    for (const auto& encuentro : ultimos_encuentros) {
        json["ultimos_encuentros"].push_back(encuentro.toJson());
    }

    return json;
}

PortadaEspecie proyectarEspecie(const Especie& especie, FechaPortada fecha) {
    PortadaEspecie salida;
    salida.id = especie.getId();
    salida.reino = especie.getReino();
    salida.nombre_comun = especie.getNombreComun();
    salida.nombre_cientifico = especie.getNombreCientifico();
    salida.foto_portada_key = especie.getFotoPortadaKey();

    // `imagenes_urls` ya viene resuelta por EspecieService; la portada se
    // queda con la primera, que es la que entra en una tarjeta.
    const auto& urls = especie.getImagenesUrls();
    if (!urls.empty()) {
        salida.foto_url = urls.front();
    }

    salida.fecha = fecha == FechaPortada::Publicacion ? especie.getCreatedAt()
                                                     : especie.getUpdatedAt();
    return salida;
}

PortadaEncuentro proyectarEncuentro(const Avistamiento& avistamiento) {
    PortadaEncuentro salida;
    salida.id = avistamiento.getId();
    salida.especie_id = avistamiento.getEspecieId();
    salida.reino = avistamiento.getReino();
    salida.nombre_sugerido = avistamiento.getNombreSugerido();
    salida.foto_key = avistamiento.getFotoKey();
    salida.foto_url = avistamiento.getFotoUrl();
    salida.creado_por = avistamiento.getCreadoPor();
    salida.observado_en = avistamiento.getObservadoEn();
    salida.created_at = avistamiento.getCreatedAt();
    return salida;
}
