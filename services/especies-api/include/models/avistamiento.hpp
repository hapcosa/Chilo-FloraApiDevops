#ifndef AVISTAMIENTO_HPP
#define AVISTAMIENTO_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

#include "identificacion.hpp"
#include "reino.hpp"

enum class AvistamientoEstado {
    Pendiente,
    Aprobado,
    Rechazado
};

std::string avistamientoEstadoToString(AvistamientoEstado estado);
AvistamientoEstado avistamientoEstadoFromString(const std::string& value);

// Quién decide qué: `visibilidad` la elige el autor, `estado` la moderación. Un
// encuentro privado no pasa por moderación porque no se ofreció a nadie.
enum class AvistamientoVisibilidad {
    Privado,
    Publico
};

std::string avistamientoVisibilidadToString(AvistamientoVisibilidad visibilidad);
AvistamientoVisibilidad avistamientoVisibilidadFromString(const std::string& value);

// Cuánto se fía la persona de la ubicación que dio. `precision_metros` mide el
// sensor; esto mide el recuerdo, y un encuentro de hace tres años solo tiene lo
// segundo (migración 0010).
enum class PrecisionDeclarada {
    Exacto,
    Aproximado,
    Zona
};

std::string precisionDeclaradaToString(PrecisionDeclarada precision);
PrecisionDeclarada precisionDeclaradaFromString(const std::string& value);

class Avistamiento {
private:
    int id = 0;
    std::optional<int> especie_id;
    Reino reino = Reino::Plantae;
    std::optional<std::string> nombre_sugerido;
    std::optional<std::string> descripcion;
    std::string foto_key;
    double geo_lat = 0;
    double geo_lng = 0;
    std::optional<double> precision_metros;
    PrecisionDeclarada precision_declarada = PrecisionDeclarada::Exacto;
    std::optional<std::string> observado_en;
    std::optional<int> creado_por;
    AvistamientoEstado estado = AvistamientoEstado::Pendiente;
    // Nace privado (ADR #12): se hace público con PATCH .../compartir, nunca
    // desde el cuerpo de un POST.
    AvistamientoVisibilidad visibilidad = AvistamientoVisibilidad::Privado;
    // Derivado de las identificaciones vigentes; el cliente no lo envía nunca
    // (ver identificacion.hpp).
    GradoIdentificacion grado_identificacion = GradoIdentificacion::SinIdentificar;
    // Cuántas identificaciones vigentes (no retiradas) tiene. Es la misma
    // evidencia de la que sale el grado, pero contada: el feed la muestra en la
    // tarjeta y sin esto haría una petición por fila.
    int identificaciones_count = 0;
    // URL de lectura firmada, resuelta por la capa de servicio al responder.
    // No es una columna: `avistamientos-fotos` es un bucket privado y la firma
    // caduca, así que guardarla no tendría sentido.
    std::optional<std::string> foto_url;
    std::optional<int> moderado_por;
    std::optional<std::string> moderado_en;
    std::optional<std::string> motivo_rechazo;
    std::optional<std::string> created_at;
    std::optional<std::string> updated_at;

public:
    int getId() const { return id; }
    const std::optional<int>& getEspecieId() const { return especie_id; }
    Reino getReino() const { return reino; }
    const std::optional<std::string>& getNombreSugerido() const { return nombre_sugerido; }
    const std::optional<std::string>& getDescripcion() const { return descripcion; }
    const std::string& getFotoKey() const { return foto_key; }
    double getGeoLat() const { return geo_lat; }
    double getGeoLng() const { return geo_lng; }
    const std::optional<double>& getPrecisionMetros() const { return precision_metros; }
    PrecisionDeclarada getPrecisionDeclarada() const { return precision_declarada; }
    const std::optional<std::string>& getObservadoEn() const { return observado_en; }
    const std::optional<int>& getCreadoPor() const { return creado_por; }
    AvistamientoEstado getEstado() const { return estado; }
    AvistamientoVisibilidad getVisibilidad() const { return visibilidad; }
    GradoIdentificacion getGradoIdentificacion() const { return grado_identificacion; }
    int getIdentificacionesCount() const { return identificaciones_count; }
    const std::optional<std::string>& getFotoUrl() const { return foto_url; }
    const std::optional<int>& getModeradoPor() const { return moderado_por; }
    const std::optional<std::string>& getModeradoEn() const { return moderado_en; }
    const std::optional<std::string>& getMotivoRechazo() const { return motivo_rechazo; }
    const std::optional<std::string>& getCreatedAt() const { return created_at; }
    const std::optional<std::string>& getUpdatedAt() const { return updated_at; }

    void setId(int value) { id = value; }
    void setEspecieId(std::optional<int> value) { especie_id = value; }
    void setReino(Reino value) { reino = value; }
    void setNombreSugerido(std::optional<std::string> value) { nombre_sugerido = std::move(value); }
    void setDescripcion(std::optional<std::string> value) { descripcion = std::move(value); }
    void setFotoKey(const std::string& value) { foto_key = value; }
    void setGeoLat(double value) { geo_lat = value; }
    void setGeoLng(double value) { geo_lng = value; }
    void setPrecisionMetros(std::optional<double> value) { precision_metros = value; }
    void setPrecisionDeclarada(PrecisionDeclarada value) { precision_declarada = value; }
    void setObservadoEn(std::optional<std::string> value) { observado_en = std::move(value); }
    void setCreadoPor(std::optional<int> value) { creado_por = value; }
    void setEstado(AvistamientoEstado value) { estado = value; }
    void setVisibilidad(AvistamientoVisibilidad value) { visibilidad = value; }
    void setGradoIdentificacion(GradoIdentificacion value) { grado_identificacion = value; }
    void setIdentificacionesCount(int value) { identificaciones_count = value; }
    void setFotoUrl(std::optional<std::string> value) { foto_url = std::move(value); }
    void setModeradoPor(std::optional<int> value) { moderado_por = value; }
    void setModeradoEn(std::optional<std::string> value) { moderado_en = std::move(value); }
    void setMotivoRechazo(std::optional<std::string> value) { motivo_rechazo = std::move(value); }
    void setCreatedAt(std::optional<std::string> value) { created_at = std::move(value); }
    void setUpdatedAt(std::optional<std::string> value) { updated_at = std::move(value); }

    bool esValido() const;
    nlohmann::json toJson() const;
    static Avistamiento fromJson(const nlohmann::json& json);
};

#endif // AVISTAMIENTO_HPP
