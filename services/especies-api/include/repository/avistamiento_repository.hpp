#ifndef AVISTAMIENTO_REPOSITORY_HPP
#define AVISTAMIENTO_REPOSITORY_HPP

#include <optional>
#include <vector>

#include "../models/avistamiento.hpp"
#include "../models/celda_mapa.hpp"

// Por qué fecha se ordena el listado.
//
// El feed ordena por `observado_en`: cuenta una historia de naturaleza, y ahí
// lo que importa es cuándo se vio el bicho. La portada ordena por `created_at`
// porque cuenta actividad de la app: desde la Fase 9 (PR 7) un encuentro puede
// ser un recuerdo de hace años, y subirlo hoy es movimiento de hoy — con
// `observado_en` no aparecería nunca.
enum class OrdenAvistamiento { ObservadoEn, CreadoEn };

struct AvistamientoFilters {
    std::optional<AvistamientoEstado> estado;
    std::optional<Reino> reino;
    std::optional<int> especie_id;
    std::optional<int> creado_por;
    std::optional<GradoIdentificacion> grado_identificacion;
    std::optional<AvistamientoVisibilidad> visibilidad;
    OrdenAvistamiento orden = OrdenAvistamiento::ObservadoEn;
    int limit = 50;
    int offset = 0;
};

// Ventana del mapa. `bbox` acota la consulta a lo que se está mirando y `zoom`
// decide el tamaño de la celda: sin ambos, la agregación sería sobre toda la
// tabla y devolvería un detalle que la pantalla no puede dibujar.
struct MapaFilters {
    double min_lat = 0;
    double min_lng = 0;
    double max_lat = 0;
    double max_lng = 0;
    int zoom = 10;
    std::optional<Reino> reino;
    std::optional<int> especie_id;
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

    // Celdas agregadas para el mapa. Solo entra lo público y aprobado: los
    // encuentros propios se piden por `find` con `creado_por`, sin agregar,
    // porque son de quien pregunta.
    virtual std::vector<CeldaMapa> mapa(const MapaFilters& filters) = 0;
};

#endif // AVISTAMIENTO_REPOSITORY_HPP

