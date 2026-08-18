// Qué fecha de observación acepta la API desde que un encuentro puede ser un
// recuerdo de hace años (Fase 9, PR 7). El reloj se pasa por parámetro para que
// el test no dependa de cuándo se ejecuta.
#include <gtest/gtest.h>

#include <ctime>
#include <string>

#include "utils/fechas.hpp"

namespace {

// 2026-08-18T12:00:00Z
constexpr std::time_t kAhora = 1787054400LL;

TEST(FechasTest, AceptaUnRecuerdoDeHaceAnios) {
    EXPECT_TRUE(utils::observadoEnEsAceptable("2019-03-14T08:30:00Z", kAhora));
    EXPECT_TRUE(utils::observadoEnEsAceptable("1974-11-02T00:00:00Z", kAhora));
}

TEST(FechasTest, AceptaSoloLaFechaSinHora) {
    // El selector del móvil puede mandar el día pelado: nadie recuerda la hora
    // de un encuentro de hace tres años.
    EXPECT_TRUE(utils::observadoEnEsAceptable("2019-03-14", kAhora));
}

TEST(FechasTest, RechazaElFuturoPeroToleraElDesfaseDelReloj) {
    // El reloj del teléfono se adelanta y su zona horaria no es la del
    // servidor: hasta 24 h por delante sigue siendo "hoy" para alguien.
    EXPECT_TRUE(utils::observadoEnEsAceptable("2026-08-19T06:00:00Z", kAhora));
    EXPECT_FALSE(utils::observadoEnEsAceptable("2026-08-21T12:00:00Z", kAhora));
    EXPECT_FALSE(utils::observadoEnEsAceptable("2100-01-01T00:00:00Z", kAhora));
}

TEST(FechasTest, RechazaAntesDelPiso) {
    // Un año de dos dígitos mal armado en el cliente cae justo aquí.
    EXPECT_FALSE(utils::observadoEnEsAceptable("1899-12-31T23:59:59Z", kAhora));
    EXPECT_FALSE(utils::observadoEnEsAceptable("0019-03-14T00:00:00Z", kAhora));
}

TEST(FechasTest, RechazaLoQueNoEsFecha) {
    EXPECT_FALSE(utils::observadoEnEsAceptable("", kAhora));
    EXPECT_FALSE(utils::observadoEnEsAceptable("ayer", kAhora));
    EXPECT_FALSE(utils::observadoEnEsAceptable("2019-13-01T00:00:00Z", kAhora));
    EXPECT_FALSE(utils::observadoEnEsAceptable("2019-03-32T00:00:00Z", kAhora));
}

} // namespace
