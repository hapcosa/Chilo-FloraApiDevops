#include "../../include/services/avistamiento_visibilidad.hpp"

#include <cmath>

#include "../../include/models/celda_mapa.hpp"

namespace {

bool esDe(const VisibilidadSolicitante& solicitante, const std::optional<int>& autor) {
    return solicitante.usuario_id && autor && *autor == *solicitante.usuario_id;
}

// Centro de la celda que contiene el punto, con la misma fórmula que la
// consulta agregada del mapa (`floor(v / tam) * tam + tam / 2`). Son dos
// escrituras de una sola regla: si divergen, el feed y el mapa contarían
// historias distintas sobre el mismo encuentro.
double centroDeCelda(double valor) {
    return std::floor(valor / kCeldaMinimaSensible) * kCeldaMinimaSensible
           + kCeldaMinimaSensible / 2;
}

} // namespace

AvistamientoFilters restringirVisibilidad(const AvistamientoFilters& filters,
                                          const VisibilidadSolicitante& solicitante) {
    if (esDe(solicitante, filters.creado_por)) {
        return filters;
    }

    AvistamientoFilters restringidos = filters;
    // Los encuentros privados son cosa de su autor y de nadie más, tampoco de
    // la moderación: nunca se ofrecieron a nadie, así que no hay nada que
    // moderar hasta que el dueño los comparta.
    restringidos.visibilidad = AvistamientoVisibilidad::Publico;
    if (!solicitante.puede_moderar) {
        restringidos.estado = AvistamientoEstado::Aprobado;
    }
    return restringidos;
}

bool puedeVerAvistamiento(const Avistamiento& avistamiento,
                          const VisibilidadSolicitante& solicitante) {
    if (esDe(solicitante, avistamiento.getCreadoPor())) {
        return true;
    }
    if (avistamiento.getVisibilidad() == AvistamientoVisibilidad::Privado) {
        return false;
    }
    return avistamiento.getEstado() == AvistamientoEstado::Aprobado ||
           solicitante.puede_moderar;
}

Avistamiento difuminarUbicacion(const Avistamiento& avistamiento,
                                const VisibilidadSolicitante& solicitante) {
    if (!avistamiento.getEspecieSensible()) {
        return avistamiento;
    }
    if (esDe(solicitante, avistamiento.getCreadoPor()) || solicitante.puede_moderar) {
        return avistamiento;
    }

    Avistamiento difuminado = avistamiento;
    difuminado.setGeoLat(centroDeCelda(avistamiento.getGeoLat()));
    difuminado.setGeoLng(centroDeCelda(avistamiento.getGeoLng()));
    // La precisión declarada por quien registró deja de describir lo que se
    // publica: decir "exacto" sobre una celda de un kilómetro sería mentir.
    difuminado.setPrecisionMetros(std::nullopt);
    difuminado.setUbicacionDifuminada(true);
    return difuminado;
}
