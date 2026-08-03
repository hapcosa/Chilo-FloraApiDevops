#ifndef POSTULACION_REPOSITORY_HPP
#define POSTULACION_REPOSITORY_HPP

#include <optional>
#include <vector>

#include "../models/postulacion_curador.hpp"

class IPostulacionRepository {
public:
    virtual ~IPostulacionRepository() = default;

    // Sin paginación: la bandeja del admin son unas pocas filas pendientes.
    // Si `estado` viene vacío devuelve todas.
    virtual std::vector<PostulacionCurador> findAll(
        std::optional<PostulacionEstado> estado) = 0;
    virtual std::vector<PostulacionCurador> findByUsuario(int usuarioId) = 0;
    virtual std::optional<PostulacionCurador> findById(int id) = 0;

    virtual PostulacionCurador create(const PostulacionCurador& postulacion) = 0;

    // Marca la postulación como aprobada **e** inserta la asignación en
    // `moderador_categorias` en la misma transacción: un aprobado sin
    // asignación dejaría al curador sin permisos y con la solicitud cerrada.
    virtual PostulacionCurador aprobar(int id, int revisadoPor) = 0;
    virtual PostulacionCurador rechazar(int id, int revisadoPor,
                                        const std::string& motivo) = 0;
};

#endif // POSTULACION_REPOSITORY_HPP
