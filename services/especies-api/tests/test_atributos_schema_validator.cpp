#include <gtest/gtest.h>

#include <stdexcept>

#include <nlohmann/json.hpp>

#include "models/reino.hpp"
#include "utils/atributos_schema_validator.hpp"

using nlohmann::json;

#ifndef SCHEMAS_DIR
#error "SCHEMAS_DIR debe definirse en CMake apuntando a config/schemas/"
#endif

namespace {

// Una sola instancia: cargar los schemas es lo que ejercita el constructor,
// y validar es const, así que compartirla entre tests es seguro.
const AtributosSchemaValidator& validator() {
    static const AtributosSchemaValidator v{SCHEMAS_DIR};
    return v;
}

}  // namespace

// El constructor debe cargar los 5 schemas sin lanzar.
TEST(AtributosSchemaValidatorTest, ConstruyeConSchemasReales) {
    EXPECT_NO_THROW(AtributosSchemaValidator{SCHEMAS_DIR});
}

TEST(AtributosSchemaValidatorTest, DirectorioInexistenteLanza) {
    EXPECT_THROW(AtributosSchemaValidator{"/no/existe/schemas"},
                 std::runtime_error);
}

// ----- Casos válidos por reino -----

TEST(AtributosSchemaValidatorTest, AnimaliaValido) {
    const json atributos = {
        {"clase", "Aves"},
        {"alimentacion", "piscivoro"},
        {"comportamiento", {{"actividad", "diurno"}, {"migratorio", true}}},
        {"reproduccion", "oviparo"},
    };
    EXPECT_NO_THROW(validator().validate(Reino::Animalia, atributos));
}

TEST(AtributosSchemaValidatorTest, PlantaeValido) {
    const json atributos = {
        {"tipo_planta", "arbol"},
        {"altura_promedio_m", 40},
        {"floracion_meses", {10, 11, 12}},
        {"usos_tradicionales", {"medicinal", "maderable"}},
    };
    EXPECT_NO_THROW(validator().validate(Reino::Plantae, atributos));
}

TEST(AtributosSchemaValidatorTest, FungiValido) {
    const json atributos = {
        {"comestibilidad", "toxico"},
        {"tipo", "agaricomiceto"},
        {"sustrato", {"madera_muerta", "suelo"}},
    };
    EXPECT_NO_THROW(validator().validate(Reino::Fungi, atributos));
}

TEST(AtributosSchemaValidatorTest, ProtistaValido) {
    const json atributos = {
        {"grupo", "algas_pardas"},
        {"ambiente", "marino"},
        {"morfologia", "talo"},
    };
    EXPECT_NO_THROW(validator().validate(Reino::Protista, atributos));
}

TEST(AtributosSchemaValidatorTest, MoneraValido) {
    const json atributos = {
        {"dominio", "bacteria"},
        {"forma", "bacilo"},
        {"gram", "negativo"},
    };
    EXPECT_NO_THROW(validator().validate(Reino::Monera, atributos));
}

// Objeto vacío: válido para reinos sin campos obligatorios.
TEST(AtributosSchemaValidatorTest, ObjetoVacioValidoEnReinosSinRequeridos) {
    EXPECT_NO_THROW(validator().validate(Reino::Animalia, json::object()));
    EXPECT_NO_THROW(validator().validate(Reino::Plantae, json::object()));
    EXPECT_NO_THROW(validator().validate(Reino::Protista, json::object()));
}

// ----- Campos obligatorios por riesgo sanitario / taxonómico -----

TEST(AtributosSchemaValidatorTest, FungiSinComestibilidadFalla) {
    const json atributos = {{"tipo", "agaricomiceto"}};
    EXPECT_THROW(validator().validate(Reino::Fungi, atributos),
                 std::invalid_argument);
    EXPECT_THROW(validator().validate(Reino::Fungi, json::object()),
                 std::invalid_argument);
}

TEST(AtributosSchemaValidatorTest, MoneraSinDominioFalla) {
    const json atributos = {{"forma", "coco"}};
    EXPECT_THROW(validator().validate(Reino::Monera, atributos),
                 std::invalid_argument);
}

// ----- additionalProperties: false en todos los reinos -----

TEST(AtributosSchemaValidatorTest, CampoDesconocidoFalla) {
    const json atributos = {{"tipo_planta", "arbol"}, {"inventado", 1}};
    EXPECT_THROW(validator().validate(Reino::Plantae, atributos),
                 std::invalid_argument);
}

// ----- Valores fuera de enum / tipo incorrecto -----

TEST(AtributosSchemaValidatorTest, EnumInvalidoFalla) {
    const json atributos = {{"comestibilidad", "delicioso"}};
    EXPECT_THROW(validator().validate(Reino::Fungi, atributos),
                 std::invalid_argument);
}

TEST(AtributosSchemaValidatorTest, TipoIncorrectoFalla) {
    // altura_promedio_m debe ser number, no string.
    const json atributos = {{"altura_promedio_m", "muy alto"}};
    EXPECT_THROW(validator().validate(Reino::Plantae, atributos),
                 std::invalid_argument);
}

TEST(AtributosSchemaValidatorTest, MesFloracionFueraDeRangoFalla) {
    const json atributos = {{"floracion_meses", {0, 13}}};
    EXPECT_THROW(validator().validate(Reino::Plantae, atributos),
                 std::invalid_argument);
}

// ----- Schema crudo servido a los clientes -----

TEST(AtributosSchemaValidatorTest, SchemaDeDevuelveElArchivoDelReino) {
    // GET /api/v1/schemas/:reino sirve esto tal cual; el panel de curaduría
    // arma el formulario con él, así que debe ser el schema real y no una copia.
    const auto& fungi = validator().schemaDe(Reino::Fungi);

    EXPECT_EQ(fungi["type"], "object");
    EXPECT_EQ(fungi["additionalProperties"], false);
    ASSERT_TRUE(fungi.contains("required"));
    EXPECT_EQ(fungi["required"], json::array({"comestibilidad"}))
        << "el disclaimer sanitario del panel depende de este required";
    EXPECT_TRUE(fungi["properties"].contains("comestibilidad"));

    // Los cinco reinos tienen schema; ninguno queda sin registrar.
    for (const auto reino : {Reino::Animalia, Reino::Plantae, Reino::Fungi,
                             Reino::Protista, Reino::Monera}) {
        EXPECT_TRUE(validator().schemaDe(reino).contains("properties"));
    }
}
