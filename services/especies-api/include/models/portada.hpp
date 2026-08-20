#ifndef PORTADA_HPP
#define PORTADA_HPP

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "avistamiento.hpp"
#include "especie.hpp"
#include "reino.hpp"

// Lo que la portada muestra de una ficha: identidad, foto y la fecha por la
// que entró a la lista. No es una Especie recortada por comodidad — es el
// contrato de la portada, que a propósito no arrastra descripción, hábitat ni
// atributos: son kilobytes que nadie lee en una tarjeta y que la ficha ya
// sirve entera cuando alguien la abre.
struct PortadaEspecie {
    int id = 0;
    Reino reino = Reino::Plantae;
    std::string nombre_comun;
    std::string nombre_cientifico;
    std::optional<std::string> foto_portada_key;
    std::optional<std::string> foto_url;
    // `created_at` en las publicadas, `updated_at` en las ediciones: es la
    // fecha por la que la lista está ordenada, no dos campos distintos según
    // el bloque. El cliente ordena por lo mismo que ordenó el servidor.
    std::optional<std::string> fecha;

    nlohmann::json toJson() const;
};

// Un encuentro de la comunidad, visto desde la portada.
//
// **Sin `geo_lat`/`geo_lng`, deliberadamente.** El mapa protege la ubicación
// de las especies amenazadas agregándolas en celdas que nunca bajan de
// kCeldaMinimaSensible (Fase 9, PR 9). Una portada que devolviera el punto
// exacto sería la puerta de atrás que deja esa protección en nada: mismo dato,
// otro endpoint. La portada no necesita coordenadas —muestra foto, especie y
// autor—, así que la forma más barata de no filtrarlas es no tenerlas.
//
// El nombre de la especie no viaja resuelto: va `especie_id` y el móvil lo
// resuelve contra su cache local, como ya hace el mapa. Evita un JOIN por
// tarjeta en el endpoint que más se abre.
struct PortadaEncuentro {
    int id = 0;
    std::optional<int> especie_id;
    Reino reino = Reino::Plantae;
    std::optional<std::string> nombre_sugerido;
    std::string foto_key;
    std::optional<std::string> foto_url;
    std::optional<int> creado_por;
    std::optional<std::string> observado_en;
    std::optional<std::string> created_at;

    nlohmann::json toJson() const;
};

struct Portada {
    std::vector<PortadaEspecie> ultimas_publicadas;
    std::vector<PortadaEspecie> ultimas_ediciones;
    std::vector<PortadaEncuentro> ultimos_encuentros;

    nlohmann::json toJson() const;
};

// Qué fecha de la ficha manda en cada bloque.
enum class FechaPortada { Publicacion, Edicion };

// Las proyecciones son funciones libres y no métodos de Especie/Avistamiento
// para que el recorte se pueda probar sin BD: son ellas, y no el controlador,
// las que garantizan que un campo nuevo en el modelo no aparezca solo en la
// portada.
PortadaEspecie proyectarEspecie(const Especie& especie, FechaPortada fecha);
PortadaEncuentro proyectarEncuentro(const Avistamiento& avistamiento);

#endif // PORTADA_HPP
