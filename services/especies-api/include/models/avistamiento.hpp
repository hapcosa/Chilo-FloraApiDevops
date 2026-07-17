#ifndef AVISTAMIENTO_HPP
#define AVISTAMIENTO_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

#include "reino.hpp"

enum class AvistamientoEstado {
    Pendiente,
    Aprobado,
    Rechazado
};

std::string avistamientoEstadoToString(AvistamientoEstado estado);
AvistamientoEstado avistamientoEstadoFromString(const std::string& value);

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
    std::optional<std::string> observado_en;
    std::optional<int> creado_por;
    AvistamientoEstado estado = AvistamientoEstado::Pendiente;
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
    const std::optional<std::string>& getObservadoEn() const { return observado_en; }
    const std::optional<int>& getCreadoPor() const { return creado_por; }
    AvistamientoEstado getEstado() const { return estado; }
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
    void setObservadoEn(std::optional<std::string> value) { observado_en = std::move(value); }
    void setCreadoPor(std::optional<int> value) { creado_por = value; }
    void setEstado(AvistamientoEstado value) { estado = value; }
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
