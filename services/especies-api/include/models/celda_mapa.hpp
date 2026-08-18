#ifndef CELDA_MAPA_HPP
#define CELDA_MAPA_HPP

#include <nlohmann/json.hpp>
#include <optional>

// Una celda del mapa de encuentros: cuántos hay dentro de un cuadrado de
// `grados` de lado, no cuáles. El endpoint agregado no devuelve filas a
// propósito —diez mil puntos ni se dibujan ni se deberían publicar de golpe—.
struct CeldaMapa {
    double lat = 0;  // centro de la celda
    double lng = 0;
    double grados = 0;
    int total = 0;
    int especies_distintas = 0;
    // Especie con más encuentros dentro de la celda. Es lo que permite rotular
    // un punto caliente ("aquí se ha visto mucho X"); el nombre lo pone el
    // cliente, que ya tiene el catálogo cacheado.
    std::optional<int> especie_dominante_id;
    // Al menos un encuentro de la celda es de una especie amenazada, así que la
    // celda ya viene difuminada a ~1 km. El cliente no debe dibujarla como un
    // punto exacto.
    bool sensible = false;

    nlohmann::json toJson() const;
};

// Lado de la celda, en grados, para cada nivel de zoom de Google Maps (0–21).
// A menos zoom, celdas más grandes: el cliente dibuja un clúster por celda y no
// tiene sentido mandarle más detalle del que la pantalla puede separar.
double gradosPorCeldaSegunZoom(int zoom);

// Piso de agregación para especies amenazadas: ~1,1 km en latitud. Una celda
// más fina que esto revelaría dónde está el ejemplar.
constexpr double kCeldaMinimaSensible = 0.01;

#endif // CELDA_MAPA_HPP
