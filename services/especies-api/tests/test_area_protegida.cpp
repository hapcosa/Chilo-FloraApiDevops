#include <gtest/gtest.h>

#include "../include/models/area_protegida.hpp"

namespace {

AreaProtegida areaValida() {
    AreaProtegida area;
    area.id = 1;
    area.nombre = "Parque Nacional Chiloé";
    area.tipo = AreaProtegidaTipo::ParqueNacional;
    area.centro_lat = -42.63;
    area.centro_lng = -74.05;
    area.min_lat = -42.90;
    area.min_lng = -74.15;
    area.max_lat = -42.15;
    area.max_lng = -73.90;
    return area;
}

} // namespace

TEST(AreaProtegidaTipoTest, RoundTripDeTodosLosTipos) {
    const AreaProtegidaTipo tipos[] = {
        AreaProtegidaTipo::ParqueNacional,   AreaProtegidaTipo::ReservaNacional,
        AreaProtegidaTipo::MonumentoNatural, AreaProtegidaTipo::SantuarioNaturaleza,
        AreaProtegidaTipo::ParquePrivado,    AreaProtegidaTipo::SitioRamsar,
        AreaProtegidaTipo::HumedalUrbano};
    for (const auto tipo : tipos) {
        EXPECT_EQ(areaProtegidaTipoFromString(areaProtegidaTipoToString(tipo)), tipo);
    }
}

TEST(AreaProtegidaTipoTest, TipoDesconocidoLanza) {
    EXPECT_THROW(areaProtegidaTipoFromString("parque_tematico"), std::invalid_argument);
}

TEST(AreaProtegidaTest, AreaConBboxCoherenteEsValida) {
    EXPECT_TRUE(areaValida().esValida());
}

TEST(AreaProtegidaTest, SinNombreNoEsValida) {
    auto area = areaValida();
    area.nombre.clear();
    EXPECT_FALSE(area.esValida());
}

TEST(AreaProtegidaTest, BboxInvertidoNoEsValido) {
    auto area = areaValida();
    std::swap(area.min_lat, area.max_lat);
    EXPECT_FALSE(area.esValida());
}

TEST(AreaProtegidaTest, CentroFueraDelBboxNoEsValido) {
    // Un pin que cae fuera de su propio parque manda al visitante a otro lado.
    auto area = areaValida();
    area.centro_lat = -41.0;
    EXPECT_FALSE(area.esValida());
}

TEST(AreaProtegidaTest, ToJsonEmiteBboxEnOrdenGeoJson) {
    const auto json = areaValida().toJson();
    ASSERT_TRUE(json["bbox"].is_array());
    ASSERT_EQ(json["bbox"].size(), 4u);
    EXPECT_DOUBLE_EQ(json["bbox"][0].get<double>(), -74.15);  // min_lng
    EXPECT_DOUBLE_EQ(json["bbox"][1].get<double>(), -42.90);  // min_lat
    EXPECT_DOUBLE_EQ(json["bbox"][2].get<double>(), -73.90);  // max_lng
    EXPECT_DOUBLE_EQ(json["bbox"][3].get<double>(), -42.15);  // max_lat
}

TEST(AreaProtegidaTest, SinGeometriaNiVerificacionLoDiceExplicitamente) {
    // El seed no trae polígonos ni datos validados; la app tiene que poder
    // mostrar "sin verificar" en vez de dar el dato por bueno.
    const auto json = areaValida().toJson();
    EXPECT_TRUE(json["geometria"].is_null());
    EXPECT_FALSE(json["verificado"].get<bool>());
}

TEST(EspecieEnAreaTest, ToJsonEmiteElReinoComoTexto) {
    EspecieEnArea especie;
    especie.especie_id = 4;
    especie.nombre_comun = "Zorro de Darwin";
    especie.nombre_cientifico = "Lycalopex fulvipes";
    especie.reino = Reino::Animalia;
    especie.avistamientos = 9;

    const auto json = especie.toJson();
    EXPECT_EQ(json["reino"].get<std::string>(), "animalia");
    EXPECT_EQ(json["avistamientos"].get<int>(), 9);
    EXPECT_TRUE(json["ultimo_avistamiento"].is_null());
}
