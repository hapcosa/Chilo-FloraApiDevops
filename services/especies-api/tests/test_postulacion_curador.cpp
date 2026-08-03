// Modelo de postulación a curador: parseo del cuerpo del cliente y las reglas
// que no delegamos en la BD.
#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

#include "models/postulacion_curador.hpp"

using json = nlohmann::json;

namespace {

json cuerpoMinimo() {
    return json{{"categoria_id", 7}, {"texto", "Anillo aves hace 12 años."}};
}

TEST(PostulacionCuradorTest, EstadoRoundTrip) {
    EXPECT_EQ(postulacionEstadoToString(PostulacionEstado::Pendiente), "pendiente");
    EXPECT_EQ(postulacionEstadoToString(PostulacionEstado::Aprobada), "aprobada");
    EXPECT_EQ(postulacionEstadoToString(PostulacionEstado::Rechazada), "rechazada");

    EXPECT_EQ(postulacionEstadoFromString("pendiente"), PostulacionEstado::Pendiente);
    EXPECT_EQ(postulacionEstadoFromString("aprobada"), PostulacionEstado::Aprobada);
    EXPECT_EQ(postulacionEstadoFromString("rechazada"), PostulacionEstado::Rechazada);
}

TEST(PostulacionCuradorTest, EstadoInventadoLanza) {
    EXPECT_THROW(postulacionEstadoFromString("aprobado"), std::invalid_argument);
    EXPECT_THROW(postulacionEstadoFromString(""), std::invalid_argument);
}

TEST(PostulacionCuradorTest, FromJsonMinimo) {
    const auto postulacion = PostulacionCurador::fromJson(cuerpoMinimo());

    EXPECT_EQ(postulacion.getCategoriaId(), 7);
    EXPECT_EQ(postulacion.getTexto(), "Anillo aves hace 12 años.");
    EXPECT_EQ(postulacion.getEstado(), PostulacionEstado::Pendiente);
}

TEST(PostulacionCuradorTest, FromJsonExigeCamposObligatorios) {
    EXPECT_THROW(PostulacionCurador::fromJson(json{{"texto", "x"}}),
                 std::invalid_argument);
    EXPECT_THROW(PostulacionCurador::fromJson(json{{"categoria_id", 7}}),
                 std::invalid_argument);
    EXPECT_THROW(
        PostulacionCurador::fromJson(json{{"categoria_id", "siete"}, {"texto", "x"}}),
        std::invalid_argument);
}

TEST(PostulacionCuradorTest, FromJsonRechazaTextoEnBlanco) {
    auto cuerpo = cuerpoMinimo();
    cuerpo["texto"] = "   \n\t ";
    EXPECT_THROW(PostulacionCurador::fromJson(cuerpo), std::invalid_argument);

    cuerpo["texto"] = "";
    EXPECT_THROW(PostulacionCurador::fromJson(cuerpo), std::invalid_argument);
}

TEST(PostulacionCuradorTest, FromJsonRechazaTextoDesmedido) {
    auto cuerpo = cuerpoMinimo();
    cuerpo["texto"] = std::string(4001, 'a');
    EXPECT_THROW(PostulacionCurador::fromJson(cuerpo), std::invalid_argument);

    cuerpo["texto"] = std::string(4000, 'a');
    EXPECT_NO_THROW(PostulacionCurador::fromJson(cuerpo));
}

TEST(PostulacionCuradorTest, FromJsonIgnoraEstadoYUsuarioDelCliente) {
    // Nadie se autoaprueba ni postula en nombre de otro mandándolo en el JSON.
    auto cuerpo = cuerpoMinimo();
    cuerpo["estado"] = "aprobada";
    cuerpo["usuario_id"] = 999;
    cuerpo["revisado_por"] = 1;

    const auto postulacion = PostulacionCurador::fromJson(cuerpo);

    EXPECT_EQ(postulacion.getEstado(), PostulacionEstado::Pendiente);
    EXPECT_EQ(postulacion.getUsuarioId(), 0);
    EXPECT_FALSE(postulacion.getRevisadoPor().has_value());
}

TEST(PostulacionCuradorTest, EsValidaExigeUsuarioYCategoria) {
    auto postulacion = PostulacionCurador::fromJson(cuerpoMinimo());
    EXPECT_FALSE(postulacion.esValida()) << "sin usuario_id no es válida";

    postulacion.setUsuarioId(10);
    EXPECT_TRUE(postulacion.esValida());

    postulacion.setCategoriaId(0);
    EXPECT_FALSE(postulacion.esValida());
}

TEST(PostulacionCuradorTest, EsValidaExigeMotivoAlRechazar) {
    auto postulacion = PostulacionCurador::fromJson(cuerpoMinimo());
    postulacion.setUsuarioId(10);
    postulacion.setEstado(PostulacionEstado::Rechazada);
    EXPECT_FALSE(postulacion.esValida());

    postulacion.setMotivo("Sin experiencia acreditada.");
    EXPECT_TRUE(postulacion.esValida());
}

TEST(PostulacionCuradorTest, ToJsonSerializaNulosComoNull) {
    auto postulacion = PostulacionCurador::fromJson(cuerpoMinimo());
    postulacion.setUsuarioId(10);

    const auto serializada = postulacion.toJson();

    EXPECT_EQ(serializada["usuario_id"], 10);
    EXPECT_EQ(serializada["categoria_id"], 7);
    EXPECT_EQ(serializada["estado"], "pendiente");
    EXPECT_TRUE(serializada["revisado_por"].is_null());
    EXPECT_TRUE(serializada["revisado_en"].is_null());
    EXPECT_TRUE(serializada["motivo"].is_null());
}

} // namespace
