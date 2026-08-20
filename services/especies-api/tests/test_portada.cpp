// Recorte de la portada. Lo que se prueba aquí no es "que se vea bonito" sino
// que el endpoint agregado no se convierta en una puerta de atrás: la portada
// es pública y sin autenticación, así que lo que sale de estas proyecciones es
// lo que ve cualquiera con la URL.
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "models/portada.hpp"

namespace {

Avistamiento encuentroDeEjemplo() {
    Avistamiento avistamiento;
    avistamiento.setId(7);
    avistamiento.setReino(Reino::Fungi);
    avistamiento.setFotoKey("avistamientos/7.jpg");
    avistamiento.setGeoLat(-42.4721);
    avistamiento.setGeoLng(-73.7658);
    avistamiento.setCreadoPor(3);
    avistamiento.setObservadoEn("2026-03-01");
    avistamiento.setCreatedAt("2026-08-18T10:00:00Z");
    return avistamiento;
}

// La razón de existir de PortadaEncuentro. El mapa redondea a celdas las
// especies amenazadas (Fase 9, PR 9); una portada que devolviera el punto
// exacto dejaría esa protección en nada por otro endpoint.
TEST(PortadaTest, EncuentroNoLlevaCoordenadas) {
    const auto json = proyectarEncuentro(encuentroDeEjemplo()).toJson();

    EXPECT_FALSE(json.contains("geo_lat"));
    EXPECT_FALSE(json.contains("geo_lng"));
    EXPECT_FALSE(json.contains("precision_metros"));

    // Y el modelo completo sí las lleva: si algún día dejara de llevarlas, este
    // test dejaría de estar comprobando nada.
    EXPECT_TRUE(encuentroDeEjemplo().toJson().contains("geo_lat"));
}

TEST(PortadaTest, EncuentroConservaLoQueLaTarjetaNecesita) {
    const auto json = proyectarEncuentro(encuentroDeEjemplo()).toJson();

    EXPECT_EQ(json["id"], 7);
    EXPECT_EQ(json["reino"], "fungi");
    EXPECT_EQ(json["foto_key"], "avistamientos/7.jpg");
    EXPECT_EQ(json["creado_por"], 3);
    EXPECT_EQ(json["created_at"], "2026-08-18T10:00:00Z");

    // Sin especie asignada ("todavía no sé cuál es") la clave sigue estando,
    // en null: el cliente no distingue "ausente" de "vacío".
    EXPECT_TRUE(json["especie_id"].is_null());
    EXPECT_TRUE(json["nombre_sugerido"].is_null());
}

Especie especieDeEjemplo() {
    Especie especie;
    especie.setId(12);
    especie.setReino(Reino::Plantae);
    especie.setNombreComun("Canelo");
    especie.setNombreCientifico("Drimys winteri");
    especie.setFotoPortadaKey(std::string("especies/12.jpg"));
    especie.setImagenesUrls({"https://minio/especies/12.jpg?firma", "https://minio/otra.jpg"});
    especie.setCreatedAt(std::string("2026-01-05T08:00:00Z"));
    especie.setUpdatedAt(std::string("2026-08-17T19:30:00Z"));
    return especie;
}

// Los dos bloques de fichas son la misma proyección con distinta fecha: la
// tarjeta dice "publicada el…" o "editada el…" y no puede leer la que no es.
TEST(PortadaTest, EspecieEligeLaFechaSegunElBloque) {
    EXPECT_EQ(proyectarEspecie(especieDeEjemplo(), FechaPortada::Publicacion).toJson()["fecha"],
              "2026-01-05T08:00:00Z");
    EXPECT_EQ(proyectarEspecie(especieDeEjemplo(), FechaPortada::Edicion).toJson()["fecha"],
              "2026-08-17T19:30:00Z");
}

// La URL firmada la resuelve EspecieService; la portada se queda con la
// primera, que es la que entra en una tarjeta.
TEST(PortadaTest, EspecieTomaLaPrimeraImagen) {
    const auto json = proyectarEspecie(especieDeEjemplo(), FechaPortada::Publicacion).toJson();

    EXPECT_EQ(json["foto_url"], "https://minio/especies/12.jpg?firma");
    EXPECT_EQ(json["nombre_cientifico"], "Drimys winteri");
    EXPECT_EQ(json["reino"], "plantae");
}

// Una ficha sin fotos no puede romper la portada ni inventarse una URL.
TEST(PortadaTest, EspecieSinImagenesDevuelveNull) {
    Especie especie = especieDeEjemplo();
    especie.setImagenesUrls({});
    especie.setFotoPortadaKey(std::nullopt);

    const auto json = proyectarEspecie(especie, FechaPortada::Publicacion).toJson();

    EXPECT_TRUE(json["foto_url"].is_null());
    EXPECT_TRUE(json["foto_portada_key"].is_null());
}

// Los tres bloques van siempre, aunque estén vacíos: la app dibuja tres
// carruseles y no tiene que preguntarse si la clave existe.
TEST(PortadaTest, PortadaVaciaTraeLosTresArrays) {
    const auto json = Portada{}.toJson();

    EXPECT_TRUE(json["ultimas_publicadas"].is_array());
    EXPECT_TRUE(json["ultimas_ediciones"].is_array());
    EXPECT_TRUE(json["ultimos_encuentros"].is_array());
    EXPECT_TRUE(json["ultimos_encuentros"].empty());
}

}  // namespace
