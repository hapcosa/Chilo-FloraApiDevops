#ifndef PORTADA_SERVICE_HPP
#define PORTADA_SERVICE_HPP

#include <memory>

#include "../models/portada.hpp"
#include "avistamiento_service.hpp"
#include "especie_service.hpp"

// Arma la portada de la app en una sola llamada.
//
// Es un compositor, no una capa de datos nueva: no habla con la BD, le pide a
// los dos servicios que ya saben resolver visibilidad y URLs firmadas. Existe
// porque la portada es lo primero que se abre, a veces con red de isla, y tres
// peticiones encadenadas ahí se notan.
//
// Todo lo que devuelve es público por construcción: solo fichas `publicada` y
// solo encuentros `aprobado` + `publico`. No recibe identidad del solicitante
// a propósito — una portada que cambiara según quién mira sería una portada
// que puede filtrar algo según quién mira.
class PortadaService {
private:
    std::shared_ptr<EspecieService> especieService;
    std::shared_ptr<AvistamientoService> avistamientoService;

public:
    PortadaService(std::shared_ptr<EspecieService> especieService,
                   std::shared_ptr<AvistamientoService> avistamientoService);

    // Cuántos elementos trae cada bloque. Acotado en el servicio y no en el
    // controlador para que el límite valga también si algún día lo llama otra
    // cosa que no sea HTTP.
    static constexpr int kLimitePorDefecto = 6;
    static constexpr int kLimiteMaximo = 20;

    Portada obtenerPortada(int limite = kLimitePorDefecto);
};

#endif // PORTADA_SERVICE_HPP
