#ifndef FAMILIA_HPP
#define FAMILIA_HPP

#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "reino.hpp"

// Modelo de familia post Fase 1. Espejo de la tabla `familias` tras
// la migración 0002_multi_reino.sql: añade el reino al que pertenece
// y un timestamp de creación.
class Familia {
private:
    int id;
    Reino reino;
    std::string nombre;
    std::string descripcion;
    std::optional<std::string> created_at;  // ISO 8601, llenado por la BD

    // Legacy: tabla familia_imagenes. Intacto en este PR; se rediseña
    // cuando exista object storage (Fase 2).
    std::string imagen_principal;
    std::vector<std::string> imagenes_urls;

public:
    Familia();

    // Getters
    int                                getId()              const { return id; }
    Reino                              getReino()           const { return reino; }
    const std::string&                 getNombre()          const { return nombre; }
    const std::string&                 getDescripcion()     const { return descripcion; }
    const std::optional<std::string>&  getCreatedAt()       const { return created_at; }

    // Setters
    void setId(int v)                              { id = v; }
    void setReino(Reino v)                         { reino = v; }
    void setNombre(const std::string& v)           { nombre = v; }
    void setDescripcion(const std::string& v)      { descripcion = v; }
    void setCreatedAt(std::optional<std::string> v){ created_at = std::move(v); }

    // Legacy: imágenes por URL.
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
    static Familia fromJson(const nlohmann::json& j);
};

#endif // FAMILIA_HPP
