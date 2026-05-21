#ifndef ESPECIE_HPP
#define ESPECIE_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <optional>

class Especie {
private:
    int id;
    std::string nombre_cientifico;
    std::string nombre_comun;
    int genero_id;
    std::string descripcion;
    std::string habitat;
    std::string distribucion;
    bool endemica;
    std::string estado_conservacion;
    std::vector<std::string> imagenes_urls;  // Cambiado de imagen_url a vector

    // Propiedades opcionales para cargar relaciones
    std::optional<std::string> genero_nombre;

public:
    // Constructores
    Especie();
    Especie(int id, const std::string& nombre_cientifico, const std::string& nombre_comun,
            int genero_id, const std::string& descripcion, const std::string& habitat,
            const std::string& distribucion, bool endemica, const std::string& estado_conservacion);

    // Getters y setters
    int getId() const;
    void setId(int id);

    std::string getNombreCientifico() const;
    void setNombreCientifico(const std::string& nombre_cientifico);

    std::string getNombreComun() const;
    void setNombreComun(const std::string& nombre_comun);

    int getGeneroId() const;
    void setGeneroId(int genero_id);

    std::string getDescripcion() const;
    void setDescripcion(const std::string& descripcion);

    // Métodos para manejar múltiples imágenes
    std::vector<std::string> getImagenesUrls() const;
    void setImagenesUrls(const std::vector<std::string>& imagenes_urls);
    void addImagenUrl(const std::string& imagen_url);
    void removeImagenUrl(const std::string& imagen_url);
    std::string getImagenPrincipal() const;  // Devuelve la primera imagen o vacío

    std::string getHabitat() const;
    void setHabitat(const std::string& habitat);

    std::string getDistribucion() const;
    void setDistribucion(const std::string& distribucion);

    bool isEndemica() const;
    void setEndemica(bool endemica);

    std::string getEstadoConservacion() const;
    void setEstadoConservacion(const std::string& estado_conservacion);

    std::optional<std::string> getGeneroNombre() const;
    void setGeneroNombre(const std::string& genero_nombre);

    // Validación
    bool esValida() const;

    // Serialización a JSON
    nlohmann::json toJson() const;
    static Especie fromJson(const nlohmann::json& j);
};

#endif // ESPECIE_HPP