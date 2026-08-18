#include <gtest/gtest.h>

#include "../include/models/celda_mapa.hpp"
#include "../include/utils/conservacion.hpp"

// El mapa público difumina la ubicación de las especies amenazadas. Lo que se
// prueba acá es la clasificación, no la agregación: el GROUP BY vive en SQL.

TEST(ConservacionTest, CategoriasUicnDeRiesgoSonSensibles) {
    EXPECT_TRUE(utils::esEstadoConservacionSensible("VU"));
    EXPECT_TRUE(utils::esEstadoConservacionSensible("EN"));
    EXPECT_TRUE(utils::esEstadoConservacionSensible("CR"));
    EXPECT_TRUE(utils::esEstadoConservacionSensible("EW"));
}

TEST(ConservacionTest, CategoriasFueraDePeligroNoSonSensibles) {
    EXPECT_FALSE(utils::esEstadoConservacionSensible("LC"));
    EXPECT_FALSE(utils::esEstadoConservacionSensible("NT"));
    EXPECT_FALSE(utils::esEstadoConservacionSensible("DD"));
}

TEST(ConservacionTest, EstadoDesconocidoNoSeConsideraSensible) {
    // Ofuscar todo lo no reconocido dejaría el mapa entero en celdas de 1 km.
    EXPECT_FALSE(utils::esEstadoConservacionSensible(""));
    EXPECT_FALSE(utils::esEstadoConservacionSensible("sin evaluar"));
}

TEST(ConservacionTest, ReconoceLaCategoriaDentroDeTextoLibre) {
    // El campo es texto libre: el panel sugiere el código pero no lo obliga.
    EXPECT_TRUE(utils::esEstadoConservacionSensible("En peligro (EN)"));
    EXPECT_TRUE(utils::esEstadoConservacionSensible("Vulnerable segun UICN 2020"));
    EXPECT_TRUE(utils::esEstadoConservacionSensible("uicn: cr"));
}

TEST(ConservacionTest, NoConfundePalabrasQueContienenElCodigo) {
    // "EN" como sílaba de otra palabra no es una categoría: si esto fallara,
    // media biblioteca quedaría difuminada.
    EXPECT_FALSE(utils::esEstadoConservacionSensible("endemica"));
    EXPECT_FALSE(utils::esEstadoConservacionSensible("criptica"));
    EXPECT_FALSE(utils::esEstadoConservacionSensible("abundante"));
}

TEST(ConservacionTest, ElPatronSqlNombraLosMismosTokens) {
    const auto patron = utils::patronSqlEstadoSensible();
    EXPECT_NE(patron.find("VU"), std::string::npos);
    EXPECT_NE(patron.find("EW"), std::string::npos);
    EXPECT_NE(patron.find("PELIGRO"), std::string::npos);
}

TEST(CeldaMapaTest, MenosZoomDaCeldasMasGrandes) {
    EXPECT_GT(gradosPorCeldaSegunZoom(5), gradosPorCeldaSegunZoom(10));
    EXPECT_GT(gradosPorCeldaSegunZoom(10), gradosPorCeldaSegunZoom(16));
    EXPECT_GT(gradosPorCeldaSegunZoom(16), gradosPorCeldaSegunZoom(21));
}

TEST(CeldaMapaTest, ZoomFueraDeRangoCaeEnLosExtremos) {
    EXPECT_DOUBLE_EQ(gradosPorCeldaSegunZoom(-3), gradosPorCeldaSegunZoom(0));
    EXPECT_DOUBLE_EQ(gradosPorCeldaSegunZoom(99), gradosPorCeldaSegunZoom(21));
}

TEST(CeldaMapaTest, ElZoomMaximoSigueSiendoMasFinoQueElPisoSensible) {
    // Si no, difuminar no cambiaría nada y la protección sería decorativa.
    EXPECT_LT(gradosPorCeldaSegunZoom(21), kCeldaMinimaSensible);
}

TEST(CeldaMapaTest, ToJsonEmiteLaCeldaCompleta) {
    CeldaMapa celda;
    celda.lat = -42.5;
    celda.lng = -73.75;
    celda.grados = 0.05;
    celda.total = 12;
    celda.especies_distintas = 3;
    celda.especie_dominante_id = 7;
    celda.sensible = true;

    const auto json = celda.toJson();
    EXPECT_DOUBLE_EQ(json["lat"].get<double>(), -42.5);
    EXPECT_DOUBLE_EQ(json["lng"].get<double>(), -73.75);
    EXPECT_DOUBLE_EQ(json["grados"].get<double>(), 0.05);
    EXPECT_EQ(json["total"].get<int>(), 12);
    EXPECT_EQ(json["especies_distintas"].get<int>(), 3);
    EXPECT_EQ(json["especie_dominante_id"].get<int>(), 7);
    EXPECT_TRUE(json["sensible"].get<bool>());
}

TEST(CeldaMapaTest, SinEspecieDominanteEmiteNull) {
    // Una celda de solo encuentros sin identificar: el cliente tiene que poder
    // distinguir "no hay especie" de "especie 0".
    CeldaMapa celda;
    celda.total = 4;
    const auto json = celda.toJson();
    EXPECT_TRUE(json["especie_dominante_id"].is_null());
}
