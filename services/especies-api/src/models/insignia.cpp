#include "../../include/models/insignia.hpp"

#include <stdexcept>

namespace {

// El motivo es lo que explica una insignia de rol ("cura la categoría Aves");
// el tope evita que se use como campo de texto libre cualquiera.
constexpr size_t kMotivoMaxLength = 500;

} // namespace

std::string insigniaTipoToString(InsigniaTipo tipo) {
    switch (tipo) {
        case InsigniaTipo::Automatica: return "automatica";
        case InsigniaTipo::Rol:        return "rol";
    }
    throw std::invalid_argument("tipo de insignia desconocido");
}

InsigniaTipo insigniaTipoFromString(const std::string& value) {
    if (value == "automatica") return InsigniaTipo::Automatica;
    if (value == "rol")        return InsigniaTipo::Rol;
    throw std::invalid_argument("'tipo' debe ser uno de: automatica, rol");
}

std::string insigniaMetricaToString(InsigniaMetrica metrica) {
    switch (metrica) {
        case InsigniaMetrica::Encuentros:           return "encuentros";
        case InsigniaMetrica::EspeciesDistintas:    return "especies_distintas";
        case InsigniaMetrica::Reinos:               return "reinos";
        case InsigniaMetrica::IdentificadoPorOtros: return "identificado_por_otros";
    }
    throw std::invalid_argument("métrica de insignia desconocida");
}

InsigniaMetrica insigniaMetricaFromString(const std::string& value) {
    if (value == "encuentros")             return InsigniaMetrica::Encuentros;
    if (value == "especies_distintas")     return InsigniaMetrica::EspeciesDistintas;
    if (value == "reinos")                 return InsigniaMetrica::Reinos;
    if (value == "identificado_por_otros") return InsigniaMetrica::IdentificadoPorOtros;
    throw std::invalid_argument(
        "'metrica' debe ser una de: encuentros, especies_distintas, reinos, "
        "identificado_por_otros");
}

nlohmann::json Insignia::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["codigo"] = codigo;
    json["nombre"] = nombre;
    json["descripcion"] = descripcion;
    json["criterio"] = criterio;
    json["tipo"] = insigniaTipoToString(tipo);
    json["metrica"] = metrica ? nlohmann::json(insigniaMetricaToString(*metrica))
                              : nlohmann::json(nullptr);
    json["umbral"] = umbral ? nlohmann::json(*umbral) : nlohmann::json(nullptr);
    return json;
}

nlohmann::json InsigniaOtorgada::toJson() const {
    nlohmann::json json = insignia.toJson();
    json["otorgada_en"] = otorgada_en;
    json["otorgada_por"] = otorgada_por ? nlohmann::json(*otorgada_por)
                                        : nlohmann::json(nullptr);
    json["motivo"] = motivo ? nlohmann::json(*motivo) : nlohmann::json(nullptr);
    return json;
}

OtorgamientoInsignia OtorgamientoInsignia::fromJson(const nlohmann::json& json) {
    OtorgamientoInsignia otorgamiento;

    if (!json.contains("usuario_id") || !json["usuario_id"].is_number_integer()) {
        throw std::invalid_argument("'usuario_id' es obligatorio y debe ser entero");
    }
    otorgamiento.usuarioId = json["usuario_id"].get<int>();
    if (otorgamiento.usuarioId <= 0) {
        throw std::invalid_argument("'usuario_id' debe ser positivo");
    }

    if (!json.contains("codigo") || !json["codigo"].is_string()) {
        throw std::invalid_argument("'codigo' es obligatorio");
    }
    otorgamiento.codigo = json["codigo"].get<std::string>();
    if (otorgamiento.codigo.empty()) {
        throw std::invalid_argument("'codigo' no puede estar vacío");
    }

    if (json.contains("motivo") && json["motivo"].is_string()) {
        const auto motivo = json["motivo"].get<std::string>();
        if (motivo.length() > kMotivoMaxLength) {
            throw std::invalid_argument("'motivo' supera los 500 caracteres");
        }
        if (!motivo.empty()) otorgamiento.motivo = motivo;
    }

    return otorgamiento;
}
