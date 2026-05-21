#ifndef ATRIBUTOS_SCHEMA_VALIDATOR_HPP
#define ATRIBUTOS_SCHEMA_VALIDATOR_HPP

#include <array>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

#include "../models/reino.hpp"

// Carga y aplica los JSON Schemas de `services/especies-api/config/schemas/`
// para validar la columna `atributos_especificos` (JSONB) según el reino
// de la especie. La validación es estricta: campos desconocidos o tipos
// incorrectos hacen fallar la operación con std::invalid_argument.
//
// Una instancia se construye en main.cpp al arrancar el servicio y se pasa
// como std::shared_ptr al EspecieService.
class AtributosSchemaValidator {
public:
    // schemasDir: ruta al directorio que contiene
    //   animalia.json, plantae.json, fungi.json, protista.json, monera.json
    // Lanza std::runtime_error si falta algún archivo o si un schema es
    // inválido. El servicio NO debe arrancar si la carga falla.
    explicit AtributosSchemaValidator(const std::string& schemasDir);

    // Valida `atributos` contra el schema del reino indicado.
    // Lanza std::invalid_argument con el mensaje del validador si la
    // validación falla. No lanza en caso normal.
    void validate(Reino reino, const nlohmann::json& atributos) const;

private:
    // Indexado por static_cast<size_t>(Reino). Tamaño 5 (los 5 reinos
    // del plan maestro; cualquier adición requiere ampliar este array
    // y los enum class de Reino simultáneamente).
    std::array<std::unique_ptr<nlohmann::json_schema::json_validator>, 5> validators_;
};

#endif  // ATRIBUTOS_SCHEMA_VALIDATOR_HPP
