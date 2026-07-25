#ifndef CATEGORIA_MODERACION_REPOSITORY_HPP
#define CATEGORIA_MODERACION_REPOSITORY_HPP

#include <optional>
#include <vector>

#include "../models/categoria_moderacion.hpp"

class ICategoriaModeracionRepository {
public:
    virtual ~ICategoriaModeracionRepository() = default;

    virtual CategoriaModeracion create(const CategoriaModeracion& categoria) = 0;
    virtual std::vector<CategoriaModeracion> findAll() = 0;
    virtual std::optional<CategoriaModeracion> findById(int id) = 0;
    virtual CategoriaModeracion update(const CategoriaModeracion& categoria) = 0;
    virtual void remove(int id) = 0;

    // Asignación muchos-a-muchos moderador <-> categoría.
    virtual void assignModerador(int categoriaId, int userId) = 0;
    virtual void unassignModerador(int categoriaId, int userId) = 0;
    virtual std::vector<int> listModeradores(int categoriaId) = 0;
    virtual bool isModeradorAssigned(int userId, int categoriaId) = 0;
};

#endif  // CATEGORIA_MODERACION_REPOSITORY_HPP
