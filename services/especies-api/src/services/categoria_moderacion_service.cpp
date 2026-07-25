#include "../../include/services/categoria_moderacion_service.hpp"

#include <stdexcept>
#include <utility>

CategoriaModeracionService::CategoriaModeracionService(
    std::shared_ptr<ICategoriaModeracionRepository> repo)
    : repository(std::move(repo)) {}

void CategoriaModeracionService::validar(const CategoriaModeracion& categoria) {
  if (!categoria.esValida()) {
    throw std::invalid_argument("'nombre' es obligatorio");
  }
}

std::vector<CategoriaModeracion> CategoriaModeracionService::getAll() {
  return repository->findAll();
}

std::optional<CategoriaModeracion> CategoriaModeracionService::findById(int id) {
  return repository->findById(id);
}

CategoriaModeracion CategoriaModeracionService::create(const CategoriaModeracion& categoria) {
  validar(categoria);
  return repository->create(categoria);
}

CategoriaModeracion CategoriaModeracionService::update(const CategoriaModeracion& categoria) {
  validar(categoria);
  return repository->update(categoria);
}

void CategoriaModeracionService::remove(int id) { repository->remove(id); }

void CategoriaModeracionService::assignModerador(int categoriaId, int userId) {
  repository->assignModerador(categoriaId, userId);
}

void CategoriaModeracionService::unassignModerador(int categoriaId, int userId) {
  repository->unassignModerador(categoriaId, userId);
}

std::vector<int> CategoriaModeracionService::listModeradores(int categoriaId) {
  return repository->listModeradores(categoriaId);
}

bool CategoriaModeracionService::isModeradorAssigned(int userId, int categoriaId) {
  return repository->isModeradorAssigned(userId, categoriaId);
}
