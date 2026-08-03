#ifndef CATEGORIA_SERVICE_HPP
#define CATEGORIA_SERVICE_HPP

#include <memory>
#include <optional>
#include <vector>

#include "../repository/categoria_repository.hpp"

class CategoriaService {
private:
    std::shared_ptr<ICategoriaRepository> repository;

    void validateCategoria(const CategoriaModeracion& categoria) const;

public:
    explicit CategoriaService(std::shared_ptr<ICategoriaRepository> repository);

    std::vector<CategoriaModeracion> getCategorias(std::optional<Reino> reino);
    std::optional<CategoriaModeracion> getCategoriaById(int id);
    CategoriaModeracion createCategoria(const CategoriaModeracion& categoria);
    CategoriaModeracion updateCategoria(int id, const CategoriaModeracion& categoria);
    void deleteCategoria(int id);
};

#endif // CATEGORIA_SERVICE_HPP
