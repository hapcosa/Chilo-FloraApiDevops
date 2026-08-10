#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "utils/timestamps.hpp"

// El caso real que rompía a Hermes: seis dígitos de fracción y offset "+00".
TEST(TimestampsTest, FormatoNativoDePostgres) {
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08.259598+00"),
              "2026-08-04T18:55:08.259Z");
}

TEST(TimestampsTest, SinFraccionAgregaMilisegundosEnCero) {
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08+00"),
              "2026-08-04T18:55:08.000Z");
}

TEST(TimestampsTest, FraccionCortaSeRellenaConCeros) {
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08.2+00"),
              "2026-08-04T18:55:08.200Z");
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08.25+00"),
              "2026-08-04T18:55:08.250Z");
}

TEST(TimestampsTest, FraccionLargaSeTruncaNoSeRedondea) {
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08.999999+00"),
              "2026-08-04T18:55:08.999Z");
}

TEST(TimestampsTest, SinDesplazamientoSeAsumeUtc) {
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08.259598"),
              "2026-08-04T18:55:08.259Z");
}

TEST(TimestampsTest, DesplazamientoNoUtcSePreservaConMinutos) {
    EXPECT_EQ(utils::toIso8601("2026-08-04 15:55:08.259598-03"),
              "2026-08-04T15:55:08.259-03:00");
    EXPECT_EQ(utils::toIso8601("2026-08-04 15:55:08-04:30"),
              "2026-08-04T15:55:08.000-04:30");
    EXPECT_EQ(utils::toIso8601("2026-08-04 15:55:08+0530"),
              "2026-08-04T15:55:08.000+05:30");
}

// Los segundos del desplazamiento (zonas históricas) se descartan porque
// ningún cliente los acepta.
TEST(TimestampsTest, SegundosDelDesplazamientoSeDescartan) {
    EXPECT_EQ(utils::toIso8601("1890-08-04 15:55:08-04:42:46"),
              "1890-08-04T15:55:08.000-04:42");
}

// Un desplazamiento de cero minutos y cero horas es UTC, se escribe como Z.
TEST(TimestampsTest, DesplazamientoCeroSeNormalizaAZ) {
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08+00:00"),
              "2026-08-04T18:55:08.000Z");
    EXPECT_EQ(utils::toIso8601("2026-08-04 18:55:08-00"),
              "2026-08-04T18:55:08.000Z");
}

// Aplicar la conversión dos veces no debe cambiar el resultado: los mapeos de
// filas y los tests de integración pueden encadenarla sin pensarlo.
TEST(TimestampsTest, EsIdempotente) {
    const std::string una_vez = utils::toIso8601("2026-08-04 18:55:08.259598+00");
    EXPECT_EQ(utils::toIso8601(una_vez), una_vez);
    EXPECT_EQ(utils::toIso8601("2026-08-04T18:55:08.259Z"),
              "2026-08-04T18:55:08.259Z");
}

// Ante algo que no es una fecha devolvemos el valor intacto en vez de
// inventar un timestamp.
TEST(TimestampsTest, ValoresNoFechaSeDevuelvenIntactos) {
    EXPECT_EQ(utils::toIso8601(""), "");
    EXPECT_EQ(utils::toIso8601("infinity"), "infinity");
    EXPECT_EQ(utils::toIso8601("-infinity"), "-infinity");
    EXPECT_EQ(utils::toIso8601("2026-08-04"), "2026-08-04");
    EXPECT_EQ(utils::toIso8601("no es una fecha"), "no es una fecha");
}

// Un desplazamiento con forma inesperada tampoco se adivina.
TEST(TimestampsTest, DesplazamientoIlegibleDevuelveElValorOriginal) {
    const std::string raro = "2026-08-04 18:55:08 BST";
    EXPECT_EQ(utils::toIso8601(raro), raro);
}

TEST(TimestampsTest, OptionalVacioSigueVacio) {
    const std::optional<std::string> vacio;
    EXPECT_FALSE(utils::toIso8601Opt(vacio).has_value());

    const std::optional<std::string> con_valor{"2026-08-04 18:55:08.259598+00"};
    EXPECT_EQ(utils::toIso8601Opt(con_valor), "2026-08-04T18:55:08.259Z");
}
