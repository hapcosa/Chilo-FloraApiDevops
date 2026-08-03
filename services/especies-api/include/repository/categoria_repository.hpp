#ifndef CATEGORIA_REPOSITORY_HPP
#define CATEGORIA_REPOSITORY_HPP

#include <optional>
#include <vector>

#include "../models/categoria_moderacion.hpp"

class ICategoriaRepository {
public:
    virtual ~ICategoriaRepository() = default;

    // Sin paginación: son unas pocas decenas de filas y el cliente las carga
    // todas para poblar selectores.
    virtual std::vector<CategoriaModeracion> findAll(std::optional<Reino> reino) = 0;
    virtual std::optional<CategoriaModeracion> findById(int id) = 0;
    virtual CategoriaModeracion create(const CategoriaModeracion& categoria) = 0;
    virtual CategoriaModeracion update(int id, const CategoriaModeracion& categoria) = 0;
    virtual void remove(int id) = 0;
};

#endif // CATEGORIA_REPOSITORY_HPP
