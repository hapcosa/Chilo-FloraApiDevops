#include <gtest/gtest.h>

#include <stdexcept>

#include "models/categoria_moderacion.hpp"

// El slug viaja en la URL y es la clave natural de la categoría. Su formato
// está duplicado en la restricción CHECK de la tabla, así que estas
// expectativas deben coincidir con
// migrations/0004_categorias_moderacion.sql.
TEST(CategoriaModeracionTest, SlugsValidos) {
    EXPECT_TRUE(esSlugValido("aves"));
    EXPECT_TRUE(esSlugValido("hongos-comestibles"));
    EXPECT_TRUE(esSlugValido("animalia-general"));
    EXPECT_TRUE(esSlugValido("aves2"));
}

TEST(CategoriaModeracionTest, SlugsInvalidos) {
    EXPECT_FALSE(esSlugValido(""));
    EXPECT_FALSE(esSlugValido("Aves"));           // mayúsculas
    EXPECT_FALSE(esSlugValido("aves rapaces"));   // espacio
    EXPECT_FALSE(esSlugValido("-aves"));          // guion inicial
    EXPECT_FALSE(esSlugValido("aves-"));          // guion final
    EXPECT_FALSE(esSlugValido("aves--rapaces"));  // guion doble
    EXPECT_FALSE(esSlugValido("aves_rapaces"));   // guion bajo
    EXPECT_FALSE(esSlugValido("añu"));            // no ASCII
    EXPECT_FALSE(esSlugValido(std::string(61, 'a')));
}

TEST(CategoriaModeracionTest, FromJsonMinimo) {
    const auto categoria = CategoriaModeracion::fromJson({
        {"slug", "aves"},
        {"nombre", "Aves"},
        {"reino", "animalia"},
    });

    EXPECT_EQ(categoria.getSlug(), "aves");
    EXPECT_EQ(categoria.getNombre(), "Aves");
    EXPECT_EQ(categoria.getReino(), Reino::Animalia);
    EXPECT_FALSE(categoria.getDescripcion().has_value());
    EXPECT_TRUE(categoria.esValida());
}

TEST(CategoriaModeracionTest, FromJsonExigeCamposObligatorios) {
    EXPECT_THROW(CategoriaModeracion::fromJson({{"nombre", "Aves"}, {"reino", "animalia"}}),
                 std::invalid_argument);
    EXPECT_THROW(CategoriaModeracion::fromJson({{"slug", "aves"}, {"reino", "animalia"}}),
                 std::invalid_argument);
    EXPECT_THROW(CategoriaModeracion::fromJson({{"slug", "aves"}, {"nombre", "Aves"}}),
                 std::invalid_argument);
}

TEST(CategoriaModeracionTest, FromJsonRechazaReinoInventado) {
    EXPECT_THROW(CategoriaModeracion::fromJson({
                     {"slug", "dragones"},
                     {"nombre", "Dragones"},
                     {"reino", "draconia"},
                 }),
                 std::invalid_argument);
}

TEST(CategoriaModeracionTest, ToJsonSerializaDescripcionNula) {
    CategoriaModeracion categoria;
    categoria.setId(7);
    categoria.setSlug("aves");
    categoria.setNombre("Aves");
    categoria.setReino(Reino::Animalia);

    const auto json = categoria.toJson();
    EXPECT_EQ(json["id"], 7);
    EXPECT_EQ(json["reino"], "animalia");
    EXPECT_TRUE(json["descripcion"].is_null());
}

TEST(CategoriaModeracionTest, EsValidaRechazaNombreVacio) {
    CategoriaModeracion categoria;
    categoria.setSlug("aves");
    categoria.setNombre("");
    EXPECT_FALSE(categoria.esValida());
}

// La app decide con este número si muestra el subgrupo en el selector, así que
// tiene que viajar siempre, incluso en cero: si faltara la clave, el cliente no
// podría distinguir "categoría vacía" de "versión vieja del backend".
TEST(CategoriaModeracionTest, ToJsonSiempreLlevaTotalEspecies) {
    CategoriaModeracion categoria;
    categoria.setSlug("animalia-peces");
    categoria.setNombre("Peces");
    categoria.setReino(Reino::Animalia);

    EXPECT_EQ(categoria.toJson()["total_especies"], 0);

    categoria.setTotalEspecies(23);
    EXPECT_EQ(categoria.toJson()["total_especies"], 23);
}
