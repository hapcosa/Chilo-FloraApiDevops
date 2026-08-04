// Identificación comunitaria: parseo del cuerpo del cliente y la regla que
// deriva el grado del avistamiento a partir de las identificaciones vigentes.
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#include "models/identificacion.hpp"

using json = nlohmann::json;

namespace {

Identificacion hecha(int id, int usuarioId, int especieId, bool decisiva = false) {
    Identificacion identificacion;
    identificacion.setId(id);
    identificacion.setAvistamientoId(1);
    identificacion.setUsuarioId(usuarioId);
    identificacion.setEspecieId(especieId);
    identificacion.setDecisiva(decisiva);
    return identificacion;
}

TEST(IdentificacionTest, GradoRoundTrip) {
    EXPECT_EQ(gradoIdentificacionToString(GradoIdentificacion::SinIdentificar),
              "sin_identificar");
    EXPECT_EQ(gradoIdentificacionToString(GradoIdentificacion::EnDiscusion),
              "en_discusion");
    EXPECT_EQ(gradoIdentificacionToString(GradoIdentificacion::Investigacion),
              "investigacion");

    EXPECT_EQ(gradoIdentificacionFromString("sin_identificar"),
              GradoIdentificacion::SinIdentificar);
    EXPECT_EQ(gradoIdentificacionFromString("en_discusion"),
              GradoIdentificacion::EnDiscusion);
    EXPECT_EQ(gradoIdentificacionFromString("investigacion"),
              GradoIdentificacion::Investigacion);
}

TEST(IdentificacionTest, GradoInventadoLanza) {
    EXPECT_THROW(gradoIdentificacionFromString("investigación"), std::invalid_argument);
    EXPECT_THROW(gradoIdentificacionFromString(""), std::invalid_argument);
}

TEST(IdentificacionTest, FromJsonMinimo) {
    const auto identificacion =
        Identificacion::fromJson(json{{"especie_id", 42}});

    EXPECT_EQ(identificacion.getEspecieId(), 42);
    EXPECT_FALSE(identificacion.getComentario().has_value());
    EXPECT_FALSE(identificacion.esDecisiva());
    EXPECT_FALSE(identificacion.estaRetirada());
}

TEST(IdentificacionTest, FromJsonExigeEspecie) {
    EXPECT_THROW(Identificacion::fromJson(json{{"comentario", "x"}}),
                 std::invalid_argument);
    EXPECT_THROW(Identificacion::fromJson(json{{"especie_id", "cuarenta"}}),
                 std::invalid_argument);
    EXPECT_THROW(Identificacion::fromJson(json{{"especie_id", 0}}),
                 std::invalid_argument);
}

TEST(IdentificacionTest, FromJsonIgnoraDecisivaYUsuarioDelCliente) {
    // Nadie se autoproclama curador ni identifica en nombre de otro.
    const auto identificacion = Identificacion::fromJson(json{{"especie_id", 42},
                                                              {"decisiva", true},
                                                              {"usuario_id", 999},
                                                              {"avistamiento_id", 7},
                                                              {"retirada", true}});

    EXPECT_FALSE(identificacion.esDecisiva());
    EXPECT_EQ(identificacion.getUsuarioId(), 0);
    EXPECT_EQ(identificacion.getAvistamientoId(), 0);
    EXPECT_FALSE(identificacion.estaRetirada());
}

TEST(IdentificacionTest, FromJsonRechazaComentarioDesmedido) {
    json cuerpo{{"especie_id", 42}};
    cuerpo["comentario"] = std::string(2001, 'a');
    EXPECT_THROW(Identificacion::fromJson(cuerpo), std::invalid_argument);

    cuerpo["comentario"] = std::string(2000, 'a');
    EXPECT_NO_THROW(Identificacion::fromJson(cuerpo));
}

TEST(CalcularGradoTest, SinIdentificacionesSinIdentificar) {
    const auto resultado = calcularGrado({});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::SinIdentificar);
    EXPECT_FALSE(resultado.especie_id.has_value());
}

TEST(CalcularGradoTest, UnaSolaNoAlcanzaQuorum) {
    const auto resultado = calcularGrado({hecha(1, 10, 42)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::EnDiscusion);
    EXPECT_FALSE(resultado.especie_id.has_value());
}

TEST(CalcularGradoTest, DosCoincidentesLleganAInvestigacion) {
    const auto resultado = calcularGrado({hecha(1, 10, 42), hecha(2, 11, 42)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::Investigacion);
    ASSERT_TRUE(resultado.especie_id.has_value());
    EXPECT_EQ(*resultado.especie_id, 42);
}

TEST(CalcularGradoTest, DosDiscrepantesSeQuedanEnDiscusion) {
    const auto resultado = calcularGrado({hecha(1, 10, 42), hecha(2, 11, 43)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::EnDiscusion);
    EXPECT_FALSE(resultado.especie_id.has_value());
}

TEST(CalcularGradoTest, DosDeTresAlcanzanElDosTercios) {
    const auto resultado =
        calcularGrado({hecha(1, 10, 42), hecha(2, 11, 42), hecha(3, 12, 43)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::Investigacion);
    ASSERT_TRUE(resultado.especie_id.has_value());
    EXPECT_EQ(*resultado.especie_id, 42);
}

TEST(CalcularGradoTest, TresDeCincoNoAlcanzanElDosTercios) {
    // 60% < 2/3. Con división entera (3/5*3 == 3) esto pasaría por error.
    const auto resultado = calcularGrado({hecha(1, 10, 42), hecha(2, 11, 42),
                                          hecha(3, 12, 42), hecha(4, 13, 43),
                                          hecha(5, 14, 44)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::EnDiscusion);
    EXPECT_FALSE(resultado.especie_id.has_value());
}

TEST(CalcularGradoTest, CuatroDeSeisAlcanzanElDosTercios) {
    const auto resultado = calcularGrado({hecha(1, 10, 42), hecha(2, 11, 42),
                                          hecha(3, 12, 42), hecha(4, 13, 42),
                                          hecha(5, 14, 43), hecha(6, 15, 44)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::Investigacion);
    ASSERT_TRUE(resultado.especie_id.has_value());
    EXPECT_EQ(*resultado.especie_id, 42);
}

TEST(CalcularGradoTest, DecisivaSolaCierraElAvistamiento) {
    const auto resultado = calcularGrado({hecha(1, 10, 99, /*decisiva=*/true)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::Investigacion);
    ASSERT_TRUE(resultado.especie_id.has_value());
    EXPECT_EQ(*resultado.especie_id, 99);
}

TEST(CalcularGradoTest, DecisivaGanaAlaMayoria) {
    const auto resultado = calcularGrado({hecha(1, 10, 42), hecha(2, 11, 42),
                                          hecha(3, 12, 99, /*decisiva=*/true)});

    EXPECT_EQ(resultado.grado, GradoIdentificacion::Investigacion);
    ASSERT_TRUE(resultado.especie_id.has_value());
    EXPECT_EQ(*resultado.especie_id, 99)
        << "el curador de la categoría manda sobre el quórum";
}

TEST(CalcularGradoTest, EntreDecisivasMandaLaMasReciente) {
    const auto resultado =
        calcularGrado({hecha(5, 10, 42, /*decisiva=*/true),
                       hecha(9, 11, 77, /*decisiva=*/true),
                       hecha(7, 12, 55, /*decisiva=*/true)});

    ASSERT_TRUE(resultado.especie_id.has_value());
    EXPECT_EQ(*resultado.especie_id, 77);
}

} // namespace
