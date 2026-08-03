#include "../../include/services/categoria_service.hpp"

#include <stdexcept>
#include <utility>

CategoriaService::CategoriaService(std::shared_ptr<ICategoriaRepository> repository)
    : repository(std::move(repository)) {}

void CategoriaService::validateCategoria(const CategoriaModeracion& categoria) const {
    if (!esSlugValido(categoria.getSlug())) {
        throw std::invalid_argument(
            "slug inválido: solo minúsculas, dígitos y guiones simples");
    }
    if (!categoria.esValida()) {
        throw std::invalid_argument("categoría inválida");
    }
}

std::vector<CategoriaModeracion> CategoriaService::getCategorias(
    std::optional<Reino> reino) {
    return repository->findAll(reino);
}

std::optional<CategoriaModeracion> CategoriaService::getCategoriaById(int id) {
    return repository->findById(id);
}

CategoriaModeracion CategoriaService::createCategoria(
    const CategoriaModeracion& categoria) {
    validateCategoria(categoria);
    return repository->create(categoria);
}

CategoriaModeracion CategoriaService::updateCategoria(
    int id,
    const CategoriaModeracion& categoria) {
    validateCategoria(categoria);
    return repository->update(id, categoria);
}

void CategoriaService::deleteCategoria(int id) {
    repository->remove(id);
}
