// familia.hpp
#ifndef FAMILIA_HPP
#define FAMILIA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class Familia
{
private:
    int id;
    std::string nombre;
    std::string descripcion;
    std::string imagen_principal;
    std::vector<std::string> imagenes_urls;

public:
    // Constructores
    Familia() = default;
    Familia(std::string nombre, std::string descripcion)
            : nombre(std::move(nombre)), descripcion(std::move(descripcion)) {}
    Familia(int id, std::string nombre, std::string descripcion)
            : id(id), nombre(nombre), descripcion(descripcion) {}
    Familia(int id, const std::string& nombre, const std::string& descripcion, const std::vector<std::string>& imagenes)
            : id(id), nombre(nombre), descripcion(descripcion),imagenes_urls(imagenes) {}

    // Getters
    int getId() const { return id; }
    const std::string &getNombre() const { return nombre; }
    const std::string &getDescripcion() const { return descripcion; }

    // Métodos para manejar imágenes
    std::vector<std::string> getImagenesUrls() const;
    void setImagenesUrls(const std::vector<std::string>& imagenes_urls);
    void addImagenUrl(const std::string& imagen_url);
    void removeImagenUrl(const std::string& imagen_url);
    std::string getImagenPrincipal() const;
    void setImagenPrincipal(const std::string& imagen_url);

    // Métodos de compatibilidad con imagen única
    std::string getImagenUrl() const;  // Para compatibilidad
    void setImagenUrl(const std::string& imagen_url);  // Para compatibilidad

    // Setters
    void setId(int newId) { id = newId; }
    void setNombre(const std::string &newNombre) { nombre = newNombre; }
    void setDescripcion(const std::string &newDescripcion) { descripcion = newDescripcion; }

    // Validación
    bool esValida() const;

    // Serialización a JSON
    nlohmann::json toJson() const;
    static Familia fromJson(const nlohmann::json &j);

};
#endif // FAMILIA_HPPILIA_HPP
