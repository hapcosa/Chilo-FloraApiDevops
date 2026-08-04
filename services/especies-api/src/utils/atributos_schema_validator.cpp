#include "../../include/utils/atributos_schema_validator.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace {

struct Entry {
    Reino reino;
    const char* file;
};

// Mapeo fijo: el orden coincide con los valores del enum class Reino.
constexpr std::array<Entry, 5> kSchemas = {{
    {Reino::Animalia, "animalia.json"},
    {Reino::Plantae,  "plantae.json"},
    {Reino::Fungi,    "fungi.json"},
    {Reino::Protista, "protista.json"},
    {Reino::Monera,   "monera.json"},
}};

nlohmann::json loadJsonFile(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        throw std::runtime_error(
            "No se pudo abrir el schema: " + path.string());
    }
    nlohmann::json j;
    try {
        stream >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "JSON inválido en " + path.string() + ": " + e.what());
    }
    return j;
}

}  // namespace

AtributosSchemaValidator::AtributosSchemaValidator(
    const std::string& schemasDir) {
    const std::filesystem::path dir(schemasDir);
    if (!std::filesystem::is_directory(dir)) {
        throw std::runtime_error(
            "Directorio de schemas no encontrado: " + dir.string());
    }

    for (const auto& entry : kSchemas) {
        const auto path = dir / entry.file;
        auto schemaJson = loadJsonFile(path);
        auto validator = std::make_unique<nlohmann::json_schema::json_validator>();
        try {
            validator->set_root_schema(schemaJson);
        } catch (const std::exception& e) {
            throw std::runtime_error(
                "Schema inválido en " + path.string() + ": " + e.what());
        }
        const auto idx = static_cast<size_t>(entry.reino);
        validators_[idx] = std::move(validator);
        schemas_[idx] = std::move(schemaJson);
    }
}

const nlohmann::json& AtributosSchemaValidator::schemaDe(Reino reino) const {
    const auto idx = static_cast<size_t>(reino);
    if (idx >= schemas_.size() || schemas_[idx].is_null()) {
        throw std::logic_error("Reino sin schema registrado");
    }
    return schemas_[idx];
}

void AtributosSchemaValidator::validate(Reino reino,
                                         const nlohmann::json& atributos) const {
    const auto idx = static_cast<size_t>(reino);
    if (idx >= validators_.size() || !validators_[idx]) {
        throw std::logic_error("Reino sin validador registrado");
    }
    try {
        validators_[idx]->validate(atributos);
    } catch (const std::exception& e) {
        throw std::invalid_argument(
            std::string("atributos_especificos no cumple el schema del reino '")
            + reinoToString(reino) + "': " + e.what());
    }
}
