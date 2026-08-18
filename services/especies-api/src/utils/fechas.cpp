#include "../../include/utils/fechas.hpp"

#include <cstdio>

namespace {

constexpr std::time_t kMargenFuturoSegundos = 24 * 60 * 60;

// 1900-01-01T00:00:00Z en tiempo Unix.
constexpr std::time_t kPisoUnix = -2208988800LL;

} // namespace

namespace utils {

bool observadoEnEsAceptable(const std::string& iso8601, std::time_t ahora) {
    std::tm tm{};
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

    // Solo la parte de fecha y hora. El desfase horario se ignora a propósito:
    // como máximo aporta ±14 h, que el margen de 24 h ya cubre, y parsearlo
    // a mano invita a errores peores que el que evita.
    const int leidos = std::sscanf(iso8601.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
                                   &year, &month, &day, &hour, &minute, &second);
    if (leidos < 3) {
        return false;
    }
    if (month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;

    // timegm y no mktime: el valor es UTC, y mktime lo interpretaría en la zona
    // del servidor, que en producción no tiene por qué ser UTC.
    const std::time_t instante = timegm(&tm);
    if (instante == static_cast<std::time_t>(-1)) {
        return false;
    }

    return instante >= kPisoUnix && instante <= ahora + kMargenFuturoSegundos;
}

} // namespace utils
