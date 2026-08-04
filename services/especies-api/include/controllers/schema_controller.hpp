#ifndef SCHEMA_CONTROLLER_HPP
#define SCHEMA_CONTROLLER_HPP

#include <memory>

#include <pistache/http.h>
#include <pistache/router.h>

#include "../utils/atributos_schema_validator.hpp"

// Sirve los JSON Schemas de `atributos_especificos` por reino. El panel de
// curaduría construye el formulario a partir de ellos: si los duplicara en el
// front, el formulario aceptaría campos que el servidor rechaza en cuanto
// alguien tocara un schema.
//
// Solo lectura y sin datos de usuario: los schemas ya están versionados en el
// repo público, así que estos endpoints no exigen sesión.
class SchemaController {
private:
    std::shared_ptr<AtributosSchemaValidator> validator;

public:
    explicit SchemaController(std::shared_ptr<AtributosSchemaValidator> validator);

    void getAll(const Pistache::Rest::Request& request,
                Pistache::Http::ResponseWriter response);

    void getByReino(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<SchemaController> controller) {
        using namespace Pistache::Rest;

        Routes::Get(router, "/api/v1/schemas",
                    Routes::bind(&SchemaController::getAll, controller));
        Routes::Get(router, "/api/v1/schemas/:reino",
                    Routes::bind(&SchemaController::getByReino, controller));
    }
};

#endif  // SCHEMA_CONTROLLER_HPP
