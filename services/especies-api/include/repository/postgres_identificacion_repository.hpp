#ifndef POSTGRES_IDENTIFICACION_REPOSITORY_HPP
#define POSTGRES_IDENTIFICACION_REPOSITORY_HPP

#include <memory>
#include <pqxx/pqxx>

#include "../utils/database.hpp"
#include "identificacion_repository.hpp"

class PostgresIdentificacionRepository : public IIdentificacionRepository {
private:
    std::shared_ptr<Database> database;

    Identificacion mapRowToIdentificacion(const pqxx::row& row);

    // Lee las vigentes, aplica la regla y escribe el grado en `avistamientos`.
    // Se llama siempre dentro de la transacción que acaba de modificar las
    // identificaciones, nunca por separado.
    void recalcularGrado(pqxx::work& txn, int avistamientoId, const ReglaGrado& regla);

public:
    explicit PostgresIdentificacionRepository(std::shared_ptr<Database> database);

    std::vector<Identificacion> findByAvistamiento(int avistamientoId) override;
    std::optional<Identificacion> findById(int id) override;
    Identificacion create(const Identificacion& identificacion,
                          const ReglaGrado& regla) override;
    std::optional<Identificacion> retirar(int id, const ReglaGrado& regla) override;
};

#endif // POSTGRES_IDENTIFICACION_REPOSITORY_HPP
