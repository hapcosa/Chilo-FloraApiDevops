#ifndef INSIGNIA_SERVICE_HPP
#define INSIGNIA_SERVICE_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../repository/insignia_repository.hpp"

class InsigniaService {
private:
    std::shared_ptr<IInsigniaRepository> repository;

public:
    explicit InsigniaService(std::shared_ptr<IInsigniaRepository> repository);

    std::vector<Insignia> getCatalogo();
    std::vector<InsigniaOtorgada> getInsigniasDe(int usuarioId);

    // Solo insignias de rol: las automáticas las gana el job o no se ganan.
    // Devuelve false si esa persona ya la tenía.
    bool otorgar(int usuarioId, const std::string& codigo, int otorgadaPor,
                 const std::optional<std::string>& motivo);
    bool revocar(int usuarioId, const std::string& codigo);

    // Devuelve cuántas insignias nuevas se otorgaron.
    int recalcular();
};

#endif // INSIGNIA_SERVICE_HPP
