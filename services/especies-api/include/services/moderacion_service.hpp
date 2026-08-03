#ifndef MODERACION_SERVICE_HPP
#define MODERACION_SERVICE_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../repository/moderador_categoria_repository.hpp"

// Decide quién puede editar qué (ADR #11 y #14). Vive fuera de
// `RequestIdentity` a propósito: la respuesta depende de la BD y no cabe en un
// struct de cabeceras HTTP.
class ModeracionService {
private:
    std::shared_ptr<IModeradorCategoriaRepository> repository;

public:
    explicit ModeracionService(std::shared_ptr<IModeradorCategoriaRepository> repository);

    // `admin` y `moderator` son globales y no necesitan asignación; el resto
    // solo puede tocar categorías que cura. Sin categoría (una ficha vieja aún
    // sin clasificar, o un POST que no la indica) solo pasan los globales:
    // nadie más tiene sobre qué demostrar permiso.
    bool puedeEditarCategoria(int usuarioId,
                              const std::string& rol,
                              std::optional<int> categoriaId);

    std::vector<CategoriaModeracion> categoriasDe(int usuarioId);

    // false si la asignación ya existía / no existía.
    bool asignarCurador(int usuarioId, int categoriaId, int asignadoPor);
    bool quitarCurador(int usuarioId, int categoriaId);
};

#endif // MODERACION_SERVICE_HPP
