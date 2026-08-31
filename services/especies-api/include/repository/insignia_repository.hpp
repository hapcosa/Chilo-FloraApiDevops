#ifndef INSIGNIA_REPOSITORY_HPP
#define INSIGNIA_REPOSITORY_HPP

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../models/insignia.hpp"

class IInsigniaRepository {
public:
    virtual ~IInsigniaRepository() = default;

    // Sin paginación: el catálogo es una docena de filas y el cliente lo carga
    // entero para pintar los íconos.
    virtual std::vector<Insignia> findAll() = 0;
    virtual std::optional<Insignia> findByCodigo(const std::string& codigo) = 0;

    virtual std::vector<InsigniaOtorgada> findByUsuario(int usuarioId) = 0;

    // Las insignias de varias personas de una sola consulta. La lista de
    // identificaciones de una ficha nombra a N usuarios distintos, y pedirlas
    // de a una sería una petición por fila. Las claves son los ids pedidos;
    // quien no tenga ninguna viene con la lista vacía, para que el cliente
    // distinga "no tiene" de "no lo pregunté" sin comparar contra su pedido.
    virtual std::map<int, std::vector<InsigniaOtorgada>> findByUsuarios(
        const std::vector<int>& usuarioIds) = 0;

    // Otorgamiento manual. false si esa persona ya la tenía: volver a
    // otorgarla no es un error, simplemente no cambia nada.
    virtual bool otorgar(int usuarioId, int insigniaId, int otorgadaPor,
                         const std::optional<std::string>& motivo) = 0;
    virtual bool revocar(int usuarioId, int insigniaId) = 0;

    // Recorre el catálogo automático y otorga lo que corresponda según la
    // actividad. Idempotente: devuelve cuántas insignias nuevas se otorgaron,
    // y correrlo dos veces seguidas da 0 la segunda. Nunca revoca: una
    // insignia ganada no se pierde porque un encuentro se despublique.
    virtual int recalcularAutomaticas() = 0;
};

#endif // INSIGNIA_REPOSITORY_HPP
