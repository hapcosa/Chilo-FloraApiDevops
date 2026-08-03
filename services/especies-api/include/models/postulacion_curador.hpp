#ifndef POSTULACION_CURADOR_HPP
#define POSTULACION_CURADOR_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

// Solicitud de un usuario para curar una categoría. Un admin la aprueba —lo
// que inserta la asignación en `moderador_categorias`— o la rechaza con un
// motivo. Espejo de la tabla `postulaciones_curador` de
// migrations/0005_postulaciones_curador.sql.
enum class PostulacionEstado {
    Pendiente,
    Aprobada,
    Rechazada
};

std::string postulacionEstadoToString(PostulacionEstado estado);
PostulacionEstado postulacionEstadoFromString(const std::string& value);

class PostulacionCurador {
private:
    int id = 0;
    int usuario_id = 0;
    int categoria_id = 0;
    std::string texto;
    PostulacionEstado estado = PostulacionEstado::Pendiente;
    std::optional<int> revisado_por;
    std::optional<std::string> revisado_en;
    std::optional<std::string> motivo;
    std::optional<std::string> created_at;
    std::optional<std::string> updated_at;

public:
    int getId() const { return id; }
    int getUsuarioId() const { return usuario_id; }
    int getCategoriaId() const { return categoria_id; }
    const std::string& getTexto() const { return texto; }
    PostulacionEstado getEstado() const { return estado; }
    const std::optional<int>& getRevisadoPor() const { return revisado_por; }
    const std::optional<std::string>& getRevisadoEn() const { return revisado_en; }
    const std::optional<std::string>& getMotivo() const { return motivo; }
    const std::optional<std::string>& getCreatedAt() const { return created_at; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at; }

    void setId(int value) { id = value; }
    void setUsuarioId(int value) { usuario_id = value; }
    void setCategoriaId(int value) { categoria_id = value; }
    void setTexto(std::string value) { texto = std::move(value); }
    void setEstado(PostulacionEstado value) { estado = value; }
    void setRevisadoPor(std::optional<int> value) { revisado_por = value; }
    void setRevisadoEn(std::optional<std::string> value) { revisado_en = std::move(value); }
    void setMotivo(std::optional<std::string> value) { motivo = std::move(value); }
    void setCreatedAt(std::optional<std::string> value) { created_at = std::move(value); }
    void setUpdatedAt(std::optional<std::string> value) { updated_at = std::move(value); }

    bool esValida() const;
    nlohmann::json toJson() const;

    // Solo lee lo que el postulante puede decidir: `categoria_id` y `texto`.
    // El usuario sale de la identidad verificada y el estado siempre nace
    // pendiente, así que nadie se autoaprueba mandando {"estado":"aprobada"}.
    static PostulacionCurador fromJson(const nlohmann::json& json);
};

#endif // POSTULACION_CURADOR_HPP
