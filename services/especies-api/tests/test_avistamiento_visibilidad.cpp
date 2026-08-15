// Quién ve qué avistamientos. `estado` es moderación de contenido, así que el
// listado comunitario no puede devolver lo que el cliente pida: la regla vive
// aislada de Pistache y de la BD para poder fijarla aquí.
#include <gtest/gtest.h>

#include <optional>

#include "services/avistamiento_visibilidad.hpp"

namespace {

VisibilidadSolicitante anonimo() {
    return VisibilidadSolicitante{};
}

VisibilidadSolicitante usuario(int id) {
    VisibilidadSolicitante solicitante;
    solicitante.usuario_id = id;
    return solicitante;
}

VisibilidadSolicitante moderador(int id) {
    VisibilidadSolicitante solicitante;
    solicitante.usuario_id = id;
    solicitante.puede_moderar = true;
    return solicitante;
}

// Por defecto público: el eje que se está probando es `estado` salvo que el
// test diga lo contrario.
Avistamiento avistamientoDe(int creadoPor,
                            AvistamientoEstado estado,
                            AvistamientoVisibilidad visibilidad =
                                AvistamientoVisibilidad::Publico) {
    Avistamiento avistamiento;
    avistamiento.setId(1);
    avistamiento.setCreadoPor(creadoPor);
    avistamiento.setEstado(estado);
    avistamiento.setVisibilidad(visibilidad);
    return avistamiento;
}

TEST(AvistamientoVisibilidadTest, SinFiltroSeAcotaAAprobado) {
    const auto filtros = restringirVisibilidad(AvistamientoFilters{}, usuario(7));

    ASSERT_TRUE(filtros.estado.has_value());
    EXPECT_EQ(*filtros.estado, AvistamientoEstado::Aprobado);
}

TEST(AvistamientoVisibilidadTest, EstadoPedidoPorUnUsuarioCorrienteSeIgnora) {
    AvistamientoFilters pedidos;
    pedidos.estado = AvistamientoEstado::Pendiente;

    const auto filtros = restringirVisibilidad(pedidos, usuario(7));

    ASSERT_TRUE(filtros.estado.has_value());
    EXPECT_EQ(*filtros.estado, AvistamientoEstado::Aprobado);
}

TEST(AvistamientoVisibilidadTest, SinSesionTambienSeAcota) {
    AvistamientoFilters pedidos;
    pedidos.estado = AvistamientoEstado::Rechazado;
    pedidos.creado_por = 7;

    const auto filtros = restringirVisibilidad(pedidos, anonimo());

    ASSERT_TRUE(filtros.estado.has_value());
    EXPECT_EQ(*filtros.estado, AvistamientoEstado::Aprobado);
}

TEST(AvistamientoVisibilidadTest, ModeradorConservaLoQuePide) {
    AvistamientoFilters pedidos;
    pedidos.estado = AvistamientoEstado::Pendiente;

    const auto filtros = restringirVisibilidad(pedidos, moderador(3));

    ASSERT_TRUE(filtros.estado.has_value());
    EXPECT_EQ(*filtros.estado, AvistamientoEstado::Pendiente);
}

TEST(AvistamientoVisibilidadTest, ModeradorSinEstadoNoSeAcota) {
    const auto filtros = restringirVisibilidad(AvistamientoFilters{}, moderador(3));

    EXPECT_FALSE(filtros.estado.has_value());
}

// "Mis encuentros" pide creado_por = yo y espera ver también los pendientes.
TEST(AvistamientoVisibilidadTest, LosPropiosSeVenEnCualquierEstado) {
    AvistamientoFilters pedidos;
    pedidos.creado_por = 7;
    pedidos.estado = AvistamientoEstado::Pendiente;

    const auto filtros = restringirVisibilidad(pedidos, usuario(7));

    ASSERT_TRUE(filtros.estado.has_value());
    EXPECT_EQ(*filtros.estado, AvistamientoEstado::Pendiente);
}

TEST(AvistamientoVisibilidadTest, LosPropiosSinEstadoTraenTodosLosSuyos) {
    AvistamientoFilters pedidos;
    pedidos.creado_por = 7;

    const auto filtros = restringirVisibilidad(pedidos, usuario(7));

    EXPECT_FALSE(filtros.estado.has_value());
}

TEST(AvistamientoVisibilidadTest, PedirLosDeOtroNoDaAcceso) {
    AvistamientoFilters pedidos;
    pedidos.creado_por = 9;
    pedidos.estado = AvistamientoEstado::Pendiente;

    const auto filtros = restringirVisibilidad(pedidos, usuario(7));

    ASSERT_TRUE(filtros.estado.has_value());
    EXPECT_EQ(*filtros.estado, AvistamientoEstado::Aprobado);
}

TEST(AvistamientoVisibilidadTest, RestringirNoTocaElRestoDeFiltros) {
    AvistamientoFilters pedidos;
    pedidos.reino = Reino::Fungi;
    pedidos.grado_identificacion = GradoIdentificacion::EnDiscusion;
    pedidos.especie_id = 42;
    pedidos.limit = 20;
    pedidos.offset = 40;

    const auto filtros = restringirVisibilidad(pedidos, usuario(7));

    ASSERT_TRUE(filtros.reino.has_value());
    EXPECT_EQ(*filtros.reino, Reino::Fungi);
    ASSERT_TRUE(filtros.grado_identificacion.has_value());
    EXPECT_EQ(*filtros.grado_identificacion, GradoIdentificacion::EnDiscusion);
    ASSERT_TRUE(filtros.especie_id.has_value());
    EXPECT_EQ(*filtros.especie_id, 42);
    EXPECT_EQ(filtros.limit, 20);
    EXPECT_EQ(filtros.offset, 40);
}

TEST(AvistamientoVisibilidadTest, FichaAprobadaLaVeCualquiera) {
    const auto avistamiento = avistamientoDe(9, AvistamientoEstado::Aprobado);

    EXPECT_TRUE(puedeVerAvistamiento(avistamiento, anonimo()));
    EXPECT_TRUE(puedeVerAvistamiento(avistamiento, usuario(7)));
}

TEST(AvistamientoVisibilidadTest, FichaSinAprobarSoloAutorYModerador) {
    const auto pendiente = avistamientoDe(9, AvistamientoEstado::Pendiente);

    EXPECT_FALSE(puedeVerAvistamiento(pendiente, anonimo()));
    EXPECT_FALSE(puedeVerAvistamiento(pendiente, usuario(7)));
    EXPECT_TRUE(puedeVerAvistamiento(pendiente, usuario(9)));
    EXPECT_TRUE(puedeVerAvistamiento(pendiente, moderador(3)));

    const auto rechazado = avistamientoDe(9, AvistamientoEstado::Rechazado);

    EXPECT_FALSE(puedeVerAvistamiento(rechazado, usuario(7)));
    EXPECT_TRUE(puedeVerAvistamiento(rechazado, usuario(9)));
    EXPECT_TRUE(puedeVerAvistamiento(rechazado, moderador(3)));
}

// creado_por es opcional en la tabla (hay filas del seed sin autor): sin autor
// no hay a quién dejar entrar, y nadie salvo moderación debe verlas.
TEST(AvistamientoVisibilidadTest, FichaSinAutorNoLaVeUnUsuarioCorriente) {
    Avistamiento sinAutor;
    sinAutor.setEstado(AvistamientoEstado::Pendiente);
    sinAutor.setVisibilidad(AvistamientoVisibilidad::Publico);

    EXPECT_FALSE(puedeVerAvistamiento(sinAutor, usuario(7)));
    EXPECT_TRUE(puedeVerAvistamiento(sinAutor, moderador(3)));
}

// --- Eje `visibilidad`: lo elige el autor, no la moderación (ADR #12) ---

TEST(AvistamientoVisibilidadTest, ElListadoAjenoSoloTraePublicos) {
    const auto filtros = restringirVisibilidad(AvistamientoFilters{}, usuario(7));

    ASSERT_TRUE(filtros.visibilidad.has_value());
    EXPECT_EQ(*filtros.visibilidad, AvistamientoVisibilidad::Publico);
}

// El moderador se salta `estado`, nunca `visibilidad`.
TEST(AvistamientoVisibilidadTest, ElModeradorTampocoListaPrivadosAjenos) {
    AvistamientoFilters pedidos;
    pedidos.visibilidad = AvistamientoVisibilidad::Privado;

    const auto filtros = restringirVisibilidad(pedidos, moderador(3));

    ASSERT_TRUE(filtros.visibilidad.has_value());
    EXPECT_EQ(*filtros.visibilidad, AvistamientoVisibilidad::Publico);
}

TEST(AvistamientoVisibilidadTest, PedirPrivadosAjenosNoDaAcceso) {
    AvistamientoFilters pedidos;
    pedidos.creado_por = 9;
    pedidos.visibilidad = AvistamientoVisibilidad::Privado;

    const auto filtros = restringirVisibilidad(pedidos, usuario(7));

    ASSERT_TRUE(filtros.visibilidad.has_value());
    EXPECT_EQ(*filtros.visibilidad, AvistamientoVisibilidad::Publico);
}

// "Mis encuentros" muestra los privados propios sin haberlos compartido.
TEST(AvistamientoVisibilidadTest, LosPropiosNoSeAcotanAPublicos) {
    AvistamientoFilters pedidos;
    pedidos.creado_por = 7;

    const auto filtros = restringirVisibilidad(pedidos, usuario(7));

    EXPECT_FALSE(filtros.visibilidad.has_value());
}

TEST(AvistamientoVisibilidadTest, FichaPrivadaSoloLaVeSuAutor) {
    const auto privado = avistamientoDe(9, AvistamientoEstado::Aprobado,
                                        AvistamientoVisibilidad::Privado);

    EXPECT_TRUE(puedeVerAvistamiento(privado, usuario(9)));
    EXPECT_FALSE(puedeVerAvistamiento(privado, usuario(7)));
    EXPECT_FALSE(puedeVerAvistamiento(privado, anonimo()));
    // Nunca se ofreció a nadie: no hay nada que moderar todavía.
    EXPECT_FALSE(puedeVerAvistamiento(privado, moderador(3)));
}

TEST(AvistamientoVisibilidadTest, FichaPrivadaSinAutorNoLaVeNadie) {
    Avistamiento sinAutor;
    sinAutor.setEstado(AvistamientoEstado::Aprobado);
    sinAutor.setVisibilidad(AvistamientoVisibilidad::Privado);

    EXPECT_FALSE(puedeVerAvistamiento(sinAutor, usuario(7)));
    EXPECT_FALSE(puedeVerAvistamiento(sinAutor, moderador(3)));
    EXPECT_FALSE(puedeVerAvistamiento(sinAutor, anonimo()));
}

// Los dos ejes son independientes: público no significa aprobado.
TEST(AvistamientoVisibilidadTest, PublicoSinAprobarSigueOculto) {
    const auto compartido = avistamientoDe(9, AvistamientoEstado::Pendiente,
                                           AvistamientoVisibilidad::Publico);

    EXPECT_FALSE(puedeVerAvistamiento(compartido, usuario(7)));
    EXPECT_TRUE(puedeVerAvistamiento(compartido, moderador(3)));
    EXPECT_TRUE(puedeVerAvistamiento(compartido, usuario(9)));
}

} // namespace
