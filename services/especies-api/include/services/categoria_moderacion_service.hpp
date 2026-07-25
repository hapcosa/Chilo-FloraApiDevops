#ifndef CATEGORIA_MODERACION_SERVICE_HPP
#define CATEGORIA_MODERACION_SERVICE_HPP

#include <memory>
#include <optional>
#include <vector>

#include "../models/categoria_moderacion.hpp"
#include "../repository/categoria_moderacion_repository.hpp"

class CategoriaModeracionService {
private:
    std::shared_ptr<ICategoriaModeracionRepository> repository;

    void validar(const CategoriaModeracion& categoria);

public:
    explicit CategoriaModeracionService(std::shared_ptr<ICategoriaModeracionRepository> repo);

    std::vector<CategoriaModeracion> getAll();
    std::optional<CategoriaModeracion> findById(int id);
    CategoriaModeracion create(const CategoriaModeracion& categoria);
    CategoriaModeracion update(const CategoriaModeracion& categoria);
    void remove(int id);

    void assignModerador(int categoriaId, int userId);
    void unassignModerador(int categoriaId, int userId);
    std::vector<int> listModeradores(int categoriaId);

    // Usado por otros controladores (Fase 2) para decidir si un moderador
    // puede editar una especie según su categoria_moderacion_id.
    bool isModeradorAssigned(int userId, int categoriaId);
};

#endif  // CATEGORIA_MODERACION_SERVICE_HPP
