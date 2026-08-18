#include "../../include/models/celda_mapa.hpp"

nlohmann::json CeldaMapa::toJson() const {
    nlohmann::json json;
    json["lat"] = lat;
    json["lng"] = lng;
    json["grados"] = grados;
    json["total"] = total;
    json["especies_distintas"] = especies_distintas;
    if (especie_dominante_id) {
        json["especie_dominante_id"] = *especie_dominante_id;
    } else {
        json["especie_dominante_id"] = nullptr;
    }
    json["sensible"] = sensible;
    return json;
}

double gradosPorCeldaSegunZoom(int zoom) {
    if (zoom <= 6) return 1.0;
    if (zoom <= 8) return 0.5;
    if (zoom <= 10) return 0.2;
    if (zoom <= 12) return 0.05;
    if (zoom <= 14) return 0.02;
    if (zoom <= 16) return 0.005;
    if (zoom <= 18) return 0.002;
    return 0.001;
}
