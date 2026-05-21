#ifndef GENERO_HPP
#define GENERO_HPP

#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Modelo de género. Tras la migración 0002, género no tiene reino propio:
// lo hereda transitivamente por la FK a familia. La clave única ahora es
// (familia_id, nombre) en vez de nombre global.
class Genero {
private:
    int id;
    std::string nombre;
    std::string descripcion;
    int familia_id;
    std::optional<std::string> created_at;

    // Legacy: imágenes por URL en tabla genero_imagenes.
    std::string imagen_principal;
    std::vector<std::string> imagenes_urls;
    std::optional<std::string> familia_nombre;

public:
    Genero();

    // Getters
    int                                getId()              const { return id; }
    const std::string&                 getNombre()          const { return nombre; }
    const std::string&                 getDescripcion()     const { return descripcion; }
    int                                getFamiliaId()       const { return familia_id; }
    const std::optional<std::string>&  getCreatedAt()       const { return created_at; }
    const std::optional<std::string>&  getFamiliaNombre()   const { return familia_nombre; }

    // Setters
    void setId(int v)                                { id = v; }
    void setNombre(const std::string& v)             { nombre = v; }
    void setDescripcion(const std::string& v)        { descripcion = v; }
    void setFamiliaId(int v)                         { familia_id = v; }
    void setCreatedAt(std::optional<std::string> v)  { created_at = std::move(v); }
    void setFamiliaNombre(const std::string& v)      { familia_nombre = v; }

    // Imágenes legacy
    std::vector<std::string> getImagenesUrls() const;
    void setImagenesUrls(const std::vector<std::string>& imagenes_urls);
    void addImagenUrl(const std::string& imagen_url);
    void removeImagenUrl(const std::string& imagen_url);
    std::string getImagenPrincipal() const;
    void setImagenPrincipal(const std::string& imagen_url);
    std::string getImagenUrl() const;
    void setImagenUrl(const std::string& imagen_url);

    bool esValida() const;
    nlohmann::json toJson() const;
    static Genero fromJson(const nlohmann::json& j);
};

#endif // GENERO_HPP
