#ifndef CATEGORIA_MODERACION_HPP
#define CATEGORIA_MODERACION_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

#include "reino.hpp"

// Unidad de asignación entre moderadores y especies. Puede ser tan amplia
// como un reino completo (reinos con poca documentación, ej. Fungi) o tan
// específica como un subgrupo dentro de un reino (ej. "Aves" dentro de
// Animalia). Toda especie pertenece a exactamente una categoría.
class CategoriaModeracion {
private:
    int id = 0;
    Reino reino = Reino::Animalia;
    std::string nombre;
    std::optional<std::string> descripcion;
    std::optional<std::string> created_at;

public:
    int getId() const { return id; }
    Reino getReino() const { return reino; }
    const std::string& getNombre() const { return nombre; }
    const std::optional<std::string>& getDescripcion() const { return descripcion; }
    const std::optional<std::string>& getCreatedAt() const { return created_at; }

    void setId(int value) { id = value; }
    void setReino(Reino value) { reino = value; }
    void setNombre(const std::string& value) { nombre = value; }
    void setDescripcion(std::optional<std::string> value) { descripcion = std::move(value); }
    void setCreatedAt(std::optional<std::string> value) { created_at = std::move(value); }

    bool esValida() const;
    nlohmann::json toJson() const;
    static CategoriaModeracion fromJson(const nlohmann::json& json);
};

#endif  // CATEGORIA_MODERACION_HPP
