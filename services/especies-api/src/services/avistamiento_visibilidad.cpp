#include "../../include/services/avistamiento_visibilidad.hpp"

namespace {

bool esDe(const VisibilidadSolicitante& solicitante, const std::optional<int>& autor) {
    return solicitante.usuario_id && autor && *autor == *solicitante.usuario_id;
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
