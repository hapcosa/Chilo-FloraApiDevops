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

// `estado` es moderación de contenido: un avistamiento pendiente o rechazado
// puede tener una foto que no debe circular. El listado es público para
// cualquier sesión, así que el estado pedido por el cliente no se respeta salvo
// que quien pregunte tenga derecho a verlo:
//
//   - admin/moderator ven lo que pidan (la cola de moderación es su trabajo);
//   - un usuario que filtra por sus propios avistamientos ve los suyos en
//     cualquier estado (es lo que usa "Mis encuentros");
//   - cualquier otro caso se fuerza a `aprobado`, sin error: el feed no tiene
//     por qué saber que pidió de más.
AvistamientoFilters restringirVisibilidad(const AvistamientoFilters& filters,
                                          const VisibilidadSolicitante& solicitante);

// La misma regla aplicada a una ficha concreta: sin esto, acotar el listado no
// serviría de nada porque los ids se enumeran a mano.
bool puedeVerAvistamiento(const Avistamiento& avistamiento,
                          const VisibilidadSolicitante& solicitante);

#endif // AVISTAMIENTO_VISIBILIDAD_HPP
