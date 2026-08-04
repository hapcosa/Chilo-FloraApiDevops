#ifndef IDENTIFICACION_REPOSITORY_HPP
#define IDENTIFICACION_REPOSITORY_HPP

#include <functional>
#include <optional>
#include <vector>

#include "../models/identificacion.hpp"

// La regla que traduce identificaciones vigentes en grado. El repositorio la
// recibe en vez de conocerla: así el cálculo vive en el modelo (testeable sin
// BD) pero se ejecuta **dentro** de la transacción que acaba de escribir, que
// es lo único que evita que dos identificaciones simultáneas dejen el grado
// calculado sobre un conteo viejo.
using ReglaGrado = std::function<ResultadoGrado(const std::vector<Identificacion>&)>;

class IIdentificacionRepository {
public:
    virtual ~IIdentificacionRepository() = default;

    // Incluye las retiradas: el historial es lo que explica cómo se llegó al
    // grado actual. Más antiguas primero.
    virtual std::vector<Identificacion> findByAvistamiento(int avistamientoId) = 0;
    virtual std::optional<Identificacion> findById(int id) = 0;

    // Inserta y recalcula el grado del avistamiento en la misma transacción.
    virtual Identificacion create(const Identificacion& identificacion,
                                  const ReglaGrado& regla) = 0;

    // Marca `retirada = true` y recalcula. Devuelve nullopt si no existe o ya
    // estaba retirada.
    virtual std::optional<Identificacion> retirar(int id, const ReglaGrado& regla) = 0;
};

#endif // IDENTIFICACION_REPOSITORY_HPP
