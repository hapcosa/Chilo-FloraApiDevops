#include <gtest/gtest.h>

#include <stdexcept>

#include "models/reino.hpp"

// reinoToString y reinoFromString deben ser inversos exactos para los 5 reinos.
TEST(ReinoTest, RoundTripTodosLosReinos) {
    const Reino reinos[] = {Reino::Animalia, Reino::Plantae, Reino::Fungi,
                            Reino::Protista, Reino::Monera};
    for (Reino r : reinos) {
        EXPECT_EQ(reinoFromString(reinoToString(r)), r);
    }
}

TEST(ReinoTest, SerializacionLowercase) {
    EXPECT_EQ(reinoToString(Reino::Animalia), "animalia");
    EXPECT_EQ(reinoToString(Reino::Plantae), "plantae");
    EXPECT_EQ(reinoToString(Reino::Fungi), "fungi");
    EXPECT_EQ(reinoToString(Reino::Protista), "protista");
    EXPECT_EQ(reinoToString(Reino::Monera), "monera");
}

TEST(ReinoTest, ParseoValido) {
    EXPECT_EQ(reinoFromString("fungi"), Reino::Fungi);
    EXPECT_EQ(reinoFromString("monera"), Reino::Monera);
}

TEST(ReinoTest, ParseoInvalidoLanza) {
    EXPECT_THROW(reinoFromString("plantas"), std::invalid_argument);
    EXPECT_THROW(reinoFromString(""), std::invalid_argument);
    // Case-sensitive: solo aceptamos lowercase.
    EXPECT_THROW(reinoFromString("Animalia"), std::invalid_argument);
}

TEST(ReinoTest, IsValidReino) {
    EXPECT_TRUE(isValidReino("animalia"));
    EXPECT_TRUE(isValidReino("plantae"));
    EXPECT_FALSE(isValidReino("hongo"));
    EXPECT_FALSE(isValidReino("FUNGI"));
    EXPECT_FALSE(isValidReino(""));
}
