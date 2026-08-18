#ifndef AREA_PROTEGIDA_HPP
#define AREA_PROTEGIDA_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "reino.hpp"

// Un parque o área protegida de Chiloé. Espejo de la tabla `areas_protegidas`
// (migrations/0012_areas_protegidas.sql).
//
// Es una entidad y no una etiqueta de los avistamientos porque tiene cosas
// propias —administrador, accesos, sitio web— y porque el recorrido que se
// quiere habilitar va al revés del catálogo: no "dónde vive esta especie" sino
// "a qué parque voy y qué puedo ver ahí".
enum class AreaProtegidaTipo {
    ParqueNacional,
    ReservaNacional,
    MonumentoNatural,
    SantuarioNaturaleza,
    ParquePrivado,
    SitioRamsar,
    HumedalUrbano
};

std::string areaProtegidaTipoToString(AreaProtegidaTipo tipo);
AreaProtegidaTipo areaProtegidaTipoFromString(const std::string& value);

struct AreaProtegida {
    int id = 0;
    std::string nombre;
    AreaProtegidaTipo tipo = AreaProtegidaTipo::ParqueNacional;
    std::optional<std::string> descripcion;
    std::optional<std::string> administrador;
    std::optional<std::string> accesos;
    std::optional<std::string> sitio_web;
    double centro_lat = 0;
    double centro_lng = 0;
    double min_lat = 0;
    double min_lng = 0;
    double max_lat = 0;
    double max_lng = 0;
    // GeoJSON del polígono, cuando curaduría lo haya cargado. Nulo mientras
    // tanto: un polígono inventado se leería como el límite real del parque.
    std::optional<nlohmann::json> geometria;
    std::optional<double> superficie_ha;
    std::optional<std::string> fuente;
    // Los datos del seed son de conocimiento público, no de fuente oficial. La
    // app tiene que poder decirlo hasta que curaduría revise la ficha.
    bool verificado = false;
    std::optional<std::string> created_at;
    std::optional<std::string> updated_at;

    bool esValida() const;
    nlohmann::json toJson() const;
};

// Qué especies se han registrado dentro de un área. Es la mitad turística de la
// ficha: el visitante llega por el parque, no por la especie.
struct EspecieEnArea {
    int especie_id = 0;
    std::string nombre_comun;
    std::string nombre_cientifico;
    Reino reino = Reino::Animalia;
    int avistamientos = 0;
    std::optional<std::string> ultimo_avistamiento;

    nlohmann::json toJson() const;
};

#endif // AREA_PROTEGIDA_HPP
