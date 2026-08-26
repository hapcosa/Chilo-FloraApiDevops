#ifndef CATEGORIA_MODERACION_HPP
#define CATEGORIA_MODERACION_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

#include "reino.hpp"

// Subgrupo curable dentro de un reino (p. ej. "Aves" dentro de animalia).
// Es el eje sobre el que se restringe quién puede editar qué: un curador
// cubre categorías, no reinos completos. Espejo de la tabla
// `categorias_moderacion` de migrations/0004_categorias_moderacion.sql.
class CategoriaModeracion {
private:
    int id = 0;
    std::string slug;
    std::string nombre;
    Reino reino = Reino::Animalia;
    std::optional<std::string> descripcion;
    std::optional<std::string> created_at;
    std::optional<std::string> updated_at;
    // Cuántas fichas publicadas cuelgan de la categoría. No es una columna:
    // lo calcula la consulta de listado. La app lo usa para no ofrecer un
    // subgrupo vacío, que sería un filtro que devuelve una pantalla en blanco.
    int total_especies = 0;

public:
    int getId() const { return id; }
    const std::string& getSlug() const { return slug; }
    const std::string& getNombre() const { return nombre; }
    Reino getReino() const { return reino; }
    const std::optional<std::string>& getDescripcion() const { return descripcion; }
    const std::optional<std::string>& getCreatedAt() const { return created_at; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at; }
    int getTotalEspecies() const { return total_especies; }

    void setId(int value) { id = value; }
    void setSlug(std::string value) { slug = std::move(value); }
    void setNombre(std::string value) { nombre = std::move(value); }
    void setReino(Reino value) { reino = value; }
    void setDescripcion(std::optional<std::string> value) { descripcion = std::move(value); }
    void setCreatedAt(std::optional<std::string> value) { created_at = std::move(value); }
    void setUpdatedAt(std::optional<std::string> value) { updated_at = std::move(value); }
    void setTotalEspecies(int value) { total_especies = value; }

    bool esValida() const;
    nlohmann::json toJson() const;
    static CategoriaModeracion fromJson(const nlohmann::json& json);
};

// Valida el formato de slug que exige la restricción CHECK de la tabla:
// minúsculas ASCII, dígitos y guiones simples entre segmentos.
bool esSlugValido(const std::string& slug);

#endif // CATEGORIA_MODERACION_HPP
