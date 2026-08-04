#ifndef IDENTIFICACION_HPP
#define IDENTIFICACION_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Propuesta de "este avistamiento es esta especie", hecha por un usuario.
// Espejo de la tabla `avistamiento_identificaciones` de
// migrations/0007_identificaciones.sql.
//
// El grado del avistamiento se deriva del conjunto de identificaciones
// vigentes (ver `calcularGrado`), no se escribe a mano.

enum class GradoIdentificacion {
    SinIdentificar,
    EnDiscusion,
    Investigacion
};

std::string gradoIdentificacionToString(GradoIdentificacion grado);
GradoIdentificacion gradoIdentificacionFromString(const std::string& value);

class Identificacion {
private:
    int id = 0;
    int avistamiento_id = 0;
    int usuario_id = 0;
    int especie_id = 0;
    std::optional<std::string> comentario;
    bool decisiva = false;
    bool retirada = false;
    std::optional<std::string> created_at;
    std::optional<std::string> updated_at;

public:
    int getId() const { return id; }
    int getAvistamientoId() const { return avistamiento_id; }
    int getUsuarioId() const { return usuario_id; }
    int getEspecieId() const { return especie_id; }
    const std::optional<std::string>& getComentario() const { return comentario; }
    bool esDecisiva() const { return decisiva; }
    bool estaRetirada() const { return retirada; }
    const std::optional<std::string>& getCreatedAt() const { return created_at; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at; }

    void setId(int value) { id = value; }
    void setAvistamientoId(int value) { avistamiento_id = value; }
    void setUsuarioId(int value) { usuario_id = value; }
    void setEspecieId(int value) { especie_id = value; }
    void setComentario(std::optional<std::string> value) { comentario = std::move(value); }
    void setDecisiva(bool value) { decisiva = value; }
    void setRetirada(bool value) { retirada = value; }
    void setCreatedAt(std::optional<std::string> value) { created_at = std::move(value); }
    void setUpdatedAt(std::optional<std::string> value) { updated_at = std::move(value); }

    nlohmann::json toJson() const;

    // Solo lee `especie_id` y `comentario`. El usuario sale de la identidad
    // verificada y `decisiva` la decide el servidor consultando la curaduría:
    // si viniera del cuerpo, cualquiera se autoproclamaría voto decisivo.
    static Identificacion fromJson(const nlohmann::json& json);
};

// Conclusión derivada de las identificaciones vigentes de un avistamiento.
struct ResultadoGrado {
    GradoIdentificacion grado = GradoIdentificacion::SinIdentificar;
    // Solo se rellena cuando el grado es `investigacion`: es la especie que
    // queda fijada en el avistamiento.
    std::optional<int> especie_id;
};

// La regla, aislada como función pura para poder testearla sin base de datos y
// cambiarla sin tocar SQL (por eso el grado no se recalcula en un trigger):
//
//   - Una identificación marcada `decisiva` (curador de la categoría) cierra el
//     avistamiento por sí sola. Manda la más reciente.
//   - Si no, hacen falta >= 2 vigentes y que >= 2/3 coincidan en la especie.
//   - Con propuestas pero sin acuerdo: `en_discusion`. Sin ninguna:
//     `sin_identificar`.
//
// Espera solo identificaciones vigentes; las retiradas no cuentan.
ResultadoGrado calcularGrado(const std::vector<Identificacion>& vigentes);

#endif // IDENTIFICACION_HPP
