#include "../../include/services/insignia_service.hpp"

#include <stdexcept>
#include <utility>

InsigniaService::InsigniaService(std::shared_ptr<IInsigniaRepository> repository)
    : repository(std::move(repository)) {}

std::vector<Insignia> InsigniaService::getCatalogo() {
    return repository->findAll();
}

std::vector<InsigniaOtorgada> InsigniaService::getInsigniasDe(int usuarioId) {
    return repository->findByUsuario(usuarioId);
}

bool InsigniaService::otorgar(int usuarioId, const std::string& codigo,
                              int otorgadaPor,
                              const std::optional<std::string>& motivo) {
    const auto insignia = repository->findByCodigo(codigo);
    if (!insignia) {
        throw std::out_of_range("insignia no encontrada");
    }

    // Otorgar "50 encuentros" a dedo vaciaría de sentido a las automáticas, que
    // además el recálculo volvería a otorgar sola.
    if (!insignia->esOtorgableAMano()) {
        throw std::invalid_argument(
            "las insignias automáticas las otorga el recálculo, no un admin");
    }

    return repository->otorgar(usuarioId, insignia->getId(), otorgadaPor, motivo);
}

bool InsigniaService::revocar(int usuarioId, const std::string& codigo) {
    const auto insignia = repository->findByCodigo(codigo);
    if (!insignia) {
        throw std::out_of_range("insignia no encontrada");
    }

    // Una automática sí se puede revocar (un encuentro fraudulento, por
    // ejemplo), pero el recálculo la devolvería si los datos siguen ahí; eso
    // es asunto de quien modera el encuentro, no de esta capa.
    return repository->revocar(usuarioId, insignia->getId());
}

int InsigniaService::recalcular() {
    return repository->recalcularAutomaticas();
}
