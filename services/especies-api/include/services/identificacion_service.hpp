#ifndef IDENTIFICACION_SERVICE_HPP
#define IDENTIFICACION_SERVICE_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../repository/avistamiento_repository.hpp"
#include "../repository/especie_repository.hpp"
#include "../repository/identificacion_repository.hpp"
#include "moderacion_service.hpp"

// Identificación comunitaria de avistamientos (ADR #14). Es independiente de
// `estado`: moderar contenido —ocultar una foto— y decidir qué especie es son
// dos decisiones distintas y las toma gente distinta.
class IdentificacionService {
private:
    std::shared_ptr<IIdentificacionRepository> repository;
    std::shared_ptr<IAvistamientoRepository> avistamientos;
    std::shared_ptr<IEspecieRepository> especies;
    std::shared_ptr<ModeracionService> moderacion;

public:
    IdentificacionService(std::shared_ptr<IIdentificacionRepository> repository,
                          std::shared_ptr<IAvistamientoRepository> avistamientos,
                          std::shared_ptr<IEspecieRepository> especies,
                          std::shared_ptr<ModeracionService> moderacion);

    std::vector<Identificacion> getIdentificaciones(int avistamientoId);
    std::optional<Identificacion> getIdentificacionById(int id);

    // `rol` sale de la identidad verificada por el gateway. Marca la
    // identificación como decisiva si quien la hace cura la categoría de esa
    // especie: el voto decisivo se decide aquí, nunca se acepta del cuerpo.
    Identificacion identificar(int avistamientoId,
                               int usuarioId,
                               const std::string& rol,
                               const Identificacion& identificacion);

    // Quién puede retirarla lo decide el controller (403), igual que en el
    // resto del servicio. Devuelve nullopt si no existe o ya estaba retirada.
    std::optional<Identificacion> retirar(int identificacionId);
};

#endif // IDENTIFICACION_SERVICE_HPP
