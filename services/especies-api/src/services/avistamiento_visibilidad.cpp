#include "../../include/services/avistamiento_visibilidad.hpp"

AvistamientoFilters restringirVisibilidad(const AvistamientoFilters& filters,
                                          const VisibilidadSolicitante& solicitante) {
    if (solicitante.puede_moderar) {
        return filters;
    }

    const bool pideLosSuyos = solicitante.usuario_id && filters.creado_por &&
                              *filters.creado_por == *solicitante.usuario_id;
    if (pideLosSuyos) {
        return filters;
    }

    AvistamientoFilters restringidos = filters;
    restringidos.estado = AvistamientoEstado::Aprobado;
    return restringidos;
}

bool puedeVerAvistamiento(const Avistamiento& avistamiento,
                          const VisibilidadSolicitante& solicitante) {
    if (avistamiento.getEstado() == AvistamientoEstado::Aprobado) {
        return true;
    }
    if (solicitante.puede_moderar) {
        return true;
    }
    return solicitante.usuario_id && avistamiento.getCreadoPor() &&
           *avistamiento.getCreadoPor() == *solicitante.usuario_id;
}
