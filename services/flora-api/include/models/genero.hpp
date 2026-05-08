#ifndef GENERO_HPP
#define GENERO_HPP

#include <string>
#include <nlohmann/json.hpp>
#include <optional>

class Genero {
private:
    int id;
    std::string nombre;
    std::string descripcion;
    int familia_id;
    std::string imagen_principal;
    std::vector<std::string> imagenes_urls;
    //propiedades
    std::optional<std::string> familia_nombre;

public:
    // Constructores
    Genero() = default;

    Genero(std::string nombre, std::string descripcion): nombre(std::move(nombre)),
                                                         descripcion(std::move(descripcion)) {
    }

    Genero(int id, std::string nombre, std::string descripcion, const int familia_id)
        : id(id), nombre(std::move(nombre)), descripcion(std::move(descripcion)), familia_id(familia_id) {
    }

    Genero(std::string nombre, std::string descripcion, const int familia_id)
        : nombre(std::move(nombre)), descripcion(std::move(descripcion)), familia_id(familia_id) {
    }

    Genero(int id, const std::string &nombre, const std::string &descripcion, const std::vector<std::string> &imagenes,
           int familia_id)
        : id(id), nombre(nombre), descripcion(descripcion), imagenes_urls(imagenes), familia_id(familia_id) {
    }

    // Getters
    int getId() const { return id; }
    const std::string &getNombre() const { return nombre; }
    const std::string &getDescripcion() const { return descripcion; }
    int getFamiliaId() const { return familia_id; }

    // Setters
    void setId(int newId) { id = newId; }
    void setNombre(const std::string &newNombre) { nombre = newNombre; }
    void setDescripcion(const std::string &newDescripcion) { descripcion = newDescripcion; }
    void setFamiliaId(int newFamiliaId) { familia_id = newFamiliaId; }

    // Métodos para manejar imágenes
    std::vector<std::string> getImagenesUrls() const;

    void setImagenesUrls(const std::vector<std::string> &imagenes_urls);

    void addImagenUrl(const std::string &imagen_url);

    void removeImagenUrl(const std::string &imagen_url);

    std::string getImagenPrincipal() const;

    void setImagenPrincipal(const std::string &imagen_url);

    std::string getImagenUrl() const; // Para compatibilidad
    void setImagenUrl(const std::string &imagen_url);

    bool esValida() const;

    // Serialización a JSON
    nlohmann::json toJson() const;

    static Genero fromJson(const nlohmann::json &j);
};

#endif // GENERO_HPP
