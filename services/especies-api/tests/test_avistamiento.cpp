// Serialización del avistamiento. Solo lo que el cliente lee y podría romperse
// sin que nadie se entere; el resto del modelo se cubre end-to-end.
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <stdexcept>

#include "models/avistamiento.hpp"

namespace {

TEST(AvistamientoTest, ToJsonIncluyeConteoDeIdentificaciones) {
    Avistamiento avistamiento;
    avistamiento.setId(1);
    avistamiento.setFotoKey("avistamientos/1.jpg");

    // El feed muestra "N identificaciones" por tarjeta: la clave tiene que
    // estar siempre, también cuando no hay ninguna.
    EXPECT_EQ(avistamiento.toJson()["identificaciones_count"], 0);

    avistamiento.setIdentificacionesCount(3);
    EXPECT_EQ(avistamiento.toJson()["identificaciones_count"], 3);
}

// El conteo lo calcula la BD, no el cliente: si llegara en el cuerpo de un POST
// se ignora.
TEST(AvistamientoTest, FromJsonIgnoraElConteo) {
    const auto avistamiento = Avistamiento::fromJson(nlohmann::json{
        {"reino", "fungi"},
        {"foto_key", "avistamientos/1.jpg"},
        {"geo_lat", -42.5},
        {"geo_lng", -73.8},
        {"identificaciones_count", 99}});

    EXPECT_EQ(avistamiento.getIdentificacionesCount(), 0);
}

// Un encuentro nace privado y solo se publica por PATCH /compartir, así que
// tampoco puede llegar en el cuerpo del POST.
TEST(AvistamientoTest, NaceEnPrivadoYFromJsonNoLoCambia) {
    const auto avistamiento = Avistamiento::fromJson(nlohmann::json{
        {"reino", "fungi"},
        {"foto_key", "avistamientos/1.jpg"},
        {"geo_lat", -42.5},
        {"geo_lng", -73.8},
        {"visibilidad", "publico"}});

    EXPECT_EQ(avistamiento.getVisibilidad(), AvistamientoVisibilidad::Privado);
    EXPECT_EQ(avistamiento.toJson()["visibilidad"], "privado");
}

TEST(AvistamientoTest, VisibilidadRoundTripDeString) {
    EXPECT_EQ(avistamientoVisibilidadToString(AvistamientoVisibilidad::Publico), "publico");
    EXPECT_EQ(avistamientoVisibilidadFromString("privado"), AvistamientoVisibilidad::Privado);
    EXPECT_THROW(avistamientoVisibilidadFromString("oculto"), std::invalid_argument);
}

} // namespace
