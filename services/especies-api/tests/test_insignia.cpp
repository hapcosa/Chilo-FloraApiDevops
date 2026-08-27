// Insignias de la Fase 9 (PR 11). Se cubre lo que no delegamos en la BD: el
// parseo del cuerpo del admin y la regla de que una insignia automática no se
// otorga a dedo. El recálculo en sí es una sentencia SQL y no se prueba aquí.
#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>

#include "services/insignia_service.hpp"

namespace {

Insignia hacerInsignia(int id, const std::string& codigo, InsigniaTipo tipo) {
    Insignia insignia;
    insignia.setId(id);
    insignia.setCodigo(codigo);
    insignia.setNombre("Nombre");
    insignia.setDescripcion("Descripción");
    insignia.setCriterio("Criterio");
    insignia.setTipo(tipo);
    if (tipo == InsigniaTipo::Automatica) {
        insignia.setMetrica(InsigniaMetrica::Encuentros);
        insignia.setUmbral(10);
    }
    return insignia;
}

class FakeInsigniaRepository : public IInsigniaRepository {
public:
    std::vector<Insignia> catalogo;
    std::map<std::pair<int, int>, std::optional<std::string>> otorgadas;
    int recalculos = 0;

    std::vector<Insignia> findAll() override { return catalogo; }

    std::optional<Insignia> findByCodigo(const std::string& codigo) override {
        for (const auto& insignia : catalogo) {
            if (insignia.getCodigo() == codigo) return insignia;
        }
        return std::nullopt;
    }

    std::vector<InsigniaOtorgada> findByUsuario(int) override { return {}; }

    bool otorgar(int usuarioId, int insigniaId, int,
                 const std::optional<std::string>& motivo) override {
        return otorgadas.emplace(std::make_pair(usuarioId, insigniaId), motivo).second;
    }

    bool revocar(int usuarioId, int insigniaId) override {
        return otorgadas.erase({usuarioId, insigniaId}) > 0;
    }

    int recalcularAutomaticas() override {
        ++recalculos;
        return 3;
    }
};

struct InsigniaServiceTest : public ::testing::Test {
    std::shared_ptr<FakeInsigniaRepository> repo =
        std::make_shared<FakeInsigniaRepository>();
    InsigniaService service{repo};

    void SetUp() override {
        repo->catalogo.push_back(hacerInsignia(1, "curador", InsigniaTipo::Rol));
        repo->catalogo.push_back(hacerInsignia(2, "observador", InsigniaTipo::Automatica));
    }
};

} // namespace

TEST(InsigniaModel, TipoYMetricaVanYVuelven) {
    EXPECT_EQ(insigniaTipoToString(InsigniaTipo::Automatica), "automatica");
    EXPECT_EQ(insigniaTipoFromString("rol"), InsigniaTipo::Rol);
    EXPECT_EQ(insigniaMetricaToString(InsigniaMetrica::IdentificadoPorOtros),
              "identificado_por_otros");
    EXPECT_EQ(insigniaMetricaFromString("especies_distintas"),
              InsigniaMetrica::EspeciesDistintas);
    EXPECT_THROW(insigniaTipoFromString("automática"), std::invalid_argument);
    EXPECT_THROW(insigniaMetricaFromString("puntos"), std::invalid_argument);
}

TEST(InsigniaModel, UnaDeRolSerializaMetricaYUmbralNulos) {
    const auto json = hacerInsignia(1, "curador", InsigniaTipo::Rol).toJson();

    EXPECT_EQ(json["codigo"], "curador");
    EXPECT_EQ(json["tipo"], "rol");
    // El cliente distingue "no tiene umbral" de "umbral 0" y pinta el criterio
    // en vez de una barra de progreso.
    EXPECT_TRUE(json["metrica"].is_null());
    EXPECT_TRUE(json["umbral"].is_null());
}

TEST(InsigniaModel, OtorgadaSerializaPlanoConDatosDelCatalogo) {
    InsigniaOtorgada otorgada;
    otorgada.setInsignia(hacerInsignia(1, "curador", InsigniaTipo::Rol));
    otorgada.setOtorgadaEn("2026-08-27T12:00:00Z");
    otorgada.setOtorgadaPor(7);
    otorgada.setMotivo("Categoría Aves");

    const auto json = otorgada.toJson();
    EXPECT_EQ(json["codigo"], "curador");
    EXPECT_EQ(json["otorgada_en"], "2026-08-27T12:00:00Z");
    EXPECT_EQ(json["otorgada_por"], 7);
    EXPECT_EQ(json["motivo"], "Categoría Aves");
}

TEST(OtorgamientoInsigniaFromJson, LeeUsuarioCodigoYMotivo) {
    const auto otorgamiento = OtorgamientoInsignia::fromJson(
        nlohmann::json{{"usuario_id", 5}, {"codigo", "curador"},
                       {"motivo", "Categoría Aves"}});

    EXPECT_EQ(otorgamiento.usuarioId, 5);
    EXPECT_EQ(otorgamiento.codigo, "curador");
    ASSERT_TRUE(otorgamiento.motivo.has_value());
    EXPECT_EQ(*otorgamiento.motivo, "Categoría Aves");
}

TEST(OtorgamientoInsigniaFromJson, MotivoVacioEsAusente) {
    const auto otorgamiento = OtorgamientoInsignia::fromJson(
        nlohmann::json{{"usuario_id", 5}, {"codigo", "curador"}, {"motivo", ""}});

    EXPECT_FALSE(otorgamiento.motivo.has_value());
}

TEST(OtorgamientoInsigniaFromJson, RechazaCuerposIncompletos) {
    EXPECT_THROW(OtorgamientoInsignia::fromJson(nlohmann::json{{"codigo", "curador"}}),
                 std::invalid_argument);
    EXPECT_THROW(OtorgamientoInsignia::fromJson(nlohmann::json{{"usuario_id", 5}}),
                 std::invalid_argument);
    EXPECT_THROW(OtorgamientoInsignia::fromJson(
                     nlohmann::json{{"usuario_id", 0}, {"codigo", "curador"}}),
                 std::invalid_argument);
    EXPECT_THROW(OtorgamientoInsignia::fromJson(
                     nlohmann::json{{"usuario_id", 5}, {"codigo", "curador"},
                                    {"motivo", std::string(501, 'x')}}),
                 std::invalid_argument);
}

TEST_F(InsigniaServiceTest, OtorgarUnaDeRolLaRegistraConSuMotivo) {
    EXPECT_TRUE(service.otorgar(5, "curador", 7, std::string("Categoría Aves")));

    const auto registrada = repo->otorgadas.find({5, 1});
    ASSERT_NE(registrada, repo->otorgadas.end());
    ASSERT_TRUE(registrada->second.has_value());
    EXPECT_EQ(*registrada->second, "Categoría Aves");
}

TEST_F(InsigniaServiceTest, OtorgarDosVecesNoEsError) {
    EXPECT_TRUE(service.otorgar(5, "curador", 7, std::nullopt));
    EXPECT_FALSE(service.otorgar(5, "curador", 7, std::nullopt));
}

TEST_F(InsigniaServiceTest, NoSeOtorganAManoLasAutomaticas) {
    EXPECT_THROW(service.otorgar(5, "observador", 7, std::nullopt),
                 std::invalid_argument);
    EXPECT_TRUE(repo->otorgadas.empty());
}

TEST_F(InsigniaServiceTest, CodigoDesconocidoEsNoEncontrado) {
    EXPECT_THROW(service.otorgar(5, "inexistente", 7, std::nullopt),
                 std::out_of_range);
    EXPECT_THROW(service.revocar(5, "inexistente"), std::out_of_range);
}

TEST_F(InsigniaServiceTest, RevocarLoQueNoSeTieneDevuelveFalse) {
    EXPECT_FALSE(service.revocar(5, "curador"));
    service.otorgar(5, "curador", 7, std::nullopt);
    EXPECT_TRUE(service.revocar(5, "curador"));
}

TEST_F(InsigniaServiceTest, RecalcularDelegaYDevuelveCuantasSeOtorgaron) {
    EXPECT_EQ(service.recalcular(), 3);
    EXPECT_EQ(repo->recalculos, 1);
}
