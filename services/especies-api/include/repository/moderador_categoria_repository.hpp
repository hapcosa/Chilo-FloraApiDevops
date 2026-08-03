#ifndef MODERADOR_CATEGORIA_REPOSITORY_HPP
#define MODERADOR_CATEGORIA_REPOSITORY_HPP

#include <vector>

#include "../models/categoria_moderacion.hpp"

// Asignaciones curador ↔ categoría (tabla `moderador_categorias`).
// `usuarioId` referencia lógicamente a `usuarios` del auth-service: sin FK,
// igual que `especies.creado_por`.
class IModeradorCategoriaRepository {
public:
    virtual ~IModeradorCategoriaRepository() = default;

    virtual bool esModeradorDe(int usuarioId, int categoriaId) = 0;

    // Categorías completas y no solo sus ids: quien pregunta (el panel, o el
    // propio usuario) siempre necesita el nombre para mostrarlo.
    virtual std::vector<CategoriaModeracion> categoriasDe(int usuarioId) = 0;

    // Devuelven false si la asignación ya existía / no existía. Lanzan
    // std::out_of_range si la categoría no existe.
    virtual bool asignar(int usuarioId, int categoriaId, int asignadoPor) = 0;
    virtual bool quitar(int usuarioId, int categoriaId) = 0;
};

#endif // MODERADOR_CATEGORIA_REPOSITORY_HPP
