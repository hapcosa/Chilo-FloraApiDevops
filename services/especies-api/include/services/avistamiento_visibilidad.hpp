#ifndef AVISTAMIENTO_VISIBILIDAD_HPP
#define AVISTAMIENTO_VISIBILIDAD_HPP

#include <optional>

#include "../repository/avistamiento_repository.hpp"

// Quién pregunta, reducido a lo que la regla necesita. Deliberadamente no es
// RequestIdentity: esa vive detrás de Pistache y la regla tiene que poder
// testearse sin levantar un servidor.
struct VisibilidadSolicitante {
    std::optional<int> usuario_id;
    bool puede_moderar = false;
};

// Dos ejes, decididos por gente distinta: `visibilidad` la elige el autor
// (ADR #12) y `estado` la moderación. El listado responde a cualquier sesión,
// así que ninguno de los dos se toma del cliente tal cual:
//
//   - quien filtra por sus propios avistamientos (`creado_por` = su id) ve los
//     suyos enteros, privados y sin aprobar incluidos: es "Mis encuentros";
//   - los privados ajenos no los ve nadie más, **tampoco la moderación**: no se
//     ofrecieron a nadie, así que no hay nada que moderar hasta que su dueño
//     los comparta;
//   - dentro de lo público, admin/moderator ven cualquier estado (la cola de
//     moderación es su trabajo) y el resto solo lo `aprobado`.
//
// Lo que sobra se acota en silencio, sin error: el feed no tiene por qué saber
// que pidió de más.
AvistamientoFilters restringirVisibilidad(const AvistamientoFilters& filters,
                                          const VisibilidadSolicitante& solicitante);

// La misma regla aplicada a una ficha concreta: sin esto, acotar el listado no
// serviría de nada porque los ids se enumeran a mano.
bool puedeVerAvistamiento(const Avistamiento& avistamiento,
                          const VisibilidadSolicitante& solicitante);

#endif // AVISTAMIENTO_VISIBILIDAD_HPP
