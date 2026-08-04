#include "../../include/models/identificacion.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <stdexcept>

namespace {

// El comentario justifica la identificación ("por la nervadura de la hoja");
// el tope evita que se use como almacén de texto arbitrario.
constexpr size_t kComentarioMaxLength = 2000;

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

std::string gradoIdentificacionToString(GradoIdentificacion grado) {
    switch (grado) {
        case GradoIdentificacion::SinIdentificar: return "sin_identificar";
        case GradoIdentificacion::EnDiscusion:    return "en_discusion";
        case GradoIdentificacion::Investigacion:  return "investigacion";
    }
    throw std::invalid_argument("grado de identificación desconocido");
}

GradoIdentificacion gradoIdentificacionFromString(const std::string& value) {
    if (value == "sin_identificar") return GradoIdentificacion::SinIdentificar;
    if (value == "en_discusion")    return GradoIdentificacion::EnDiscusion;
    if (value == "investigacion")   return GradoIdentificacion::Investigacion;
    throw std::invalid_argument(
        "'grado_identificacion' debe ser uno de: sin_identificar, en_discusion, investigacion");
}

nlohmann::json Identificacion::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["avistamiento_id"] = avistamiento_id;
    json["usuario_id"] = usuario_id;
    json["especie_id"] = especie_id;
    writeOpt(json, "comentario", comentario);
    json["decisiva"] = decisiva;
    json["retirada"] = retirada;
    writeOpt(json, "created_at", created_at);
    writeOpt(json, "updated_at", updated_at);
    return json;
}

Identificacion Identificacion::fromJson(const nlohmann::json& json) {
    Identificacion identificacion;

    if (!json.contains("especie_id") || !json["especie_id"].is_number_integer()) {
        throw std::invalid_argument("'especie_id' es obligatorio y debe ser entero");
    }
    identificacion.especie_id = json["especie_id"].get<int>();
    if (identificacion.especie_id <= 0) {
        throw std::invalid_argument("'especie_id' debe ser positivo");
    }

    if (json.contains("comentario") && !json["comentario"].is_null()) {
        if (!json["comentario"].is_string()) {
            throw std::invalid_argument("'comentario' debe ser string");
        }
        const auto comentario = json["comentario"].get<std::string>();
        if (comentario.length() > kComentarioMaxLength) {
            throw std::invalid_argument("'comentario' supera los 2000 caracteres");
        }
        // Un comentario en blanco es lo mismo que no haberlo escrito.
        if (!esBlanco(comentario)) {
            identificacion.comentario = comentario;
        }
    }

    return identificacion;
}

ResultadoGrado calcularGrado(const std::vector<Identificacion>& vigentes) {
    if (vigentes.empty()) return {};

    // El voto del curador cierra el asunto sin esperar quórum: es lo que hace
    // utilizable el sistema mientras la comunidad sea chica y el quórum no se
    // cumpla nunca (ADR #14). Si hay varias decisivas manda la última, que es
    // la corrección más reciente de quien tiene autoridad sobre la categoría.
    const Identificacion* decisiva = nullptr;
    for (const auto& identificacion : vigentes) {
        if (!identificacion.esDecisiva()) continue;
        if (decisiva == nullptr || identificacion.getId() > decisiva->getId()) {
            decisiva = &identificacion;
        }
    }
    if (decisiva != nullptr) {
        return {GradoIdentificacion::Investigacion, decisiva->getEspecieId()};
    }

    if (vigentes.size() < 2) {
        return {GradoIdentificacion::EnDiscusion, std::nullopt};
    }

    std::map<int, size_t> votos;
    for (const auto& identificacion : vigentes) {
        ++votos[identificacion.getEspecieId()];
    }

    const auto masVotada = std::max_element(
        votos.begin(), votos.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    // >= 2/3 sin dividir: 3*votos >= 2*total evita el redondeo de la división
    // entera, que con 5 identificaciones dejaría pasar 3 (60%).
    if (3 * masVotada->second >= 2 * vigentes.size()) {
        return {GradoIdentificacion::Investigacion, masVotada->first};
    }

    return {GradoIdentificacion::EnDiscusion, std::nullopt};
}
