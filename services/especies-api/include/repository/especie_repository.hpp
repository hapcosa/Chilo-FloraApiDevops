#ifndef ESPECIE_REPOSITORY_HPP
#define ESPECIE_REPOSITORY_HPP

#include <vector>
#include <optional>
#include "../models/especie.hpp"
#include "especie_filters.hpp"

// Interfaz del repositorio (patrón repository)
class IEspecieRepository {
public:
    virtual ~IEspecieRepository() = default;

    // Búsqueda unificada con filtros + paginación. Devuelve datos y total
    // que cumple los filtros (sin aplicar limit/offset) para que el
    // cliente pueda paginar correctamente.
    virtual EspecieSearchResult find(const EspecieFilters& filters) = 0;

    // Métodos puntuales que no encajan en find() (lookup directo).
    virtual std::optional<Especie> findById(int id) = 0;
    virtual std::optional<Especie> getByNombreCientifico(const std::string& nombre) = 0;
    virtual Especie create(const Especie& especie) = 0;
    virtual Especie update(const Especie& especie) = 0;

    // Transición editorial aislada del update completo: cambiar de estado no
    // debe arrastrar el resto de la ficha (ni pisarla con un cuerpo viejo).
    // `publicadoPor` es nullopt al despublicar. Lanza std::out_of_range si la
    // especie no existe.
    virtual Especie setEstado(int id,
                              EspecieEstado estado,
                              std::optional<int> publicadoPor) = 0;

    virtual bool remove(int id) = 0;
    virtual bool agregarImagen(int especie_id, const std::string& imagen_url, bool es_principal = false) = 0;
    virtual bool eliminarImagen(int especie_id, const std::string& imagen_url) = 0;
    virtual std::vector<std::string> getImagenes(int especie_id) = 0;
    virtual bool setImagenPrincipal(int especie_id, const std::string& imagen_url) = 0;

};

#endif // ESPECIE_REPOSITORY_HPP