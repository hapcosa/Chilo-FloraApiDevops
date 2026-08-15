#ifndef AVISTAMIENTO_REPOSITORY_HPP
#define AVISTAMIENTO_REPOSITORY_HPP

#include <optional>
#include <vector>

#include "../models/avistamiento.hpp"

struct AvistamientoFilters {
    std::optional<AvistamientoEstado> estado;
    std::optional<Reino> reino;
    std::optional<int> especie_id;
    std::optional<int> creado_por;
    std::optional<GradoIdentificacion> grado_identificacion;
    std::optional<AvistamientoVisibilidad> visibilidad;
    int limit = 50;
    int offset = 0;
};

struct AvistamientoSearchResult {
    std::vector<Avistamiento> data;
    int total = 0;
};

struct ModeracionAvistamiento {
    AvistamientoEstado estado = AvistamientoEstado::Pendiente;
    std::optional<int> moderado_por;
    std::optional<std::string> motivo_rechazo;
};

class IAvistamientoRepository {
public:
    virtual ~IAvistamientoRepository() = default;

    virtual Avistamiento create(const Avistamiento& avistamiento) = 0;
    virtual AvistamientoSearchResult find(const AvistamientoFilters& filters) = 0;
    virtual std::optional<Avistamiento> findById(int id) = 0;
    virtual Avistamiento moderate(int id, const ModeracionAvistamiento& moderacion) = 0;

    // Publica un encuentro privado. Solo su autor: el id de usuario va en la
    // consulta, no comprobado antes, para que no exista la ventana entre
    // "comprobé que es tuyo" y "lo publiqué". nullopt si no existe o no es suyo.
    virtual std::optional<Avistamiento> compartir(int id, int usuarioId) = 0;
};

#endif // AVISTAMIENTO_REPOSITORY_HPP

