#include "services/portada_service.hpp"

#include <algorithm>

PortadaService::PortadaService(std::shared_ptr<EspecieService> especieService,
                               std::shared_ptr<AvistamientoService> avistamientoService)
    : especieService(std::move(especieService)),
      avistamientoService(std::move(avistamientoService)) {}

Portada PortadaService::obtenerPortada(int limite) {
    const int n = std::clamp(limite, 1, kLimiteMaximo);

    Portada portada;

    // Solo fichas publicadas: `visibilidad` se queda en su default (verTodo
    // false, sin categorías curadas), que es "nada de borradores". La portada
    // es pública y no sabe quién pregunta.
    EspecieFilters publicadas;
    publicadas.estado = EspecieEstado::Publicada;
    publicadas.orderby = "created_at";
    publicadas.orderdir = "desc";
    publicadas.limit = n;
    for (const auto& especie : especieService->searchEspecies(publicadas).data) {
        portada.ultimas_publicadas.push_back(
            proyectarEspecie(especie, FechaPortada::Publicacion));
    }

    EspecieFilters ediciones = publicadas;
    ediciones.orderby = "updated_at";
    for (const auto& especie : especieService->searchEspecies(ediciones).data) {
        portada.ultimas_ediciones.push_back(
            proyectarEspecie(especie, FechaPortada::Edicion));
    }

    // Aprobado + público es la misma pareja de condiciones que deja entrar al
    // mapa. No se pasa por restringirVisibilidad porque no hay solicitante:
    // aquí no existe el caso "mis encuentros", que es lo único que esa regla
    // relaja.
    AvistamientoFilters encuentros;
    encuentros.estado = AvistamientoEstado::Aprobado;
    encuentros.visibilidad = AvistamientoVisibilidad::Publico;
    encuentros.orden = OrdenAvistamiento::CreadoEn;
    encuentros.limit = n;
    for (const auto& avistamiento :
         avistamientoService->searchAvistamientos(encuentros).data) {
        portada.ultimos_encuentros.push_back(proyectarEncuentro(avistamiento));
    }

    return portada;
}
