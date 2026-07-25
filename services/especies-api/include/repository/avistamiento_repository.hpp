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
    int limit = 50;
    int offset = 0;

    // Contexto del usuario que hace la consulta (no un filtro que el
    // cliente elige): controla qué avistamientos privados puede ver. Un
    // admin ve todo; cualquier otro usuario solo ve los públicos más los
    // suyos propios, aunque sean privados.
    std::optional<int> viewerUserId;
    bool viewerIsAdmin = false;
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

    // Marca un avistamiento privado como público (entra a la cola de
    // moderación pública). Solo el dueño (userId == creado_por) puede
    // hacerlo. Lanza std::invalid_argument si no existe o no es el dueño.
    virtual Avistamiento compartir(int id, int userId) = 0;
};

#endif // AVISTAMIENTO_REPOSITORY_HPP

