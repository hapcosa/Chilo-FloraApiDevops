#ifndef INSIGNIA_HPP
#define INSIGNIA_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

// Insignias de la Fase 9 (PR 11). Espejo de las tablas `insignias` y
// `usuario_insignias` de migrations/0014_insignias.sql.
//
// Las automáticas llevan su criterio como datos (`metrica` + `umbral`) para
// que el recálculo sea una consulta única sobre todo el catálogo; las de rol
// las otorga un admin a mano y no tienen umbral que evaluar.
enum class InsigniaTipo {
    Automatica,
    Rol
};

enum class InsigniaMetrica {
    Encuentros,
    EspeciesDistintas,
    Reinos,
    IdentificadoPorOtros
};

std::string insigniaTipoToString(InsigniaTipo tipo);
InsigniaTipo insigniaTipoFromString(const std::string& value);
std::string insigniaMetricaToString(InsigniaMetrica metrica);
InsigniaMetrica insigniaMetricaFromString(const std::string& value);

class Insignia {
private:
    int id = 0;
    std::string codigo;
    std::string nombre;
    std::string descripcion;
    std::string criterio;
    InsigniaTipo tipo = InsigniaTipo::Rol;
    std::optional<InsigniaMetrica> metrica;
    std::optional<int> umbral;

public:
    int getId() const { return id; }
    const std::string& getCodigo() const { return codigo; }
    const std::string& getNombre() const { return nombre; }
    const std::string& getDescripcion() const { return descripcion; }
    const std::string& getCriterio() const { return criterio; }
    InsigniaTipo getTipo() const { return tipo; }
    const std::optional<InsigniaMetrica>& getMetrica() const { return metrica; }
    const std::optional<int>& getUmbral() const { return umbral; }

    void setId(int value) { id = value; }
    void setCodigo(std::string value) { codigo = std::move(value); }
    void setNombre(std::string value) { nombre = std::move(value); }
    void setDescripcion(std::string value) { descripcion = std::move(value); }
    void setCriterio(std::string value) { criterio = std::move(value); }
    void setTipo(InsigniaTipo value) { tipo = value; }
    void setMetrica(std::optional<InsigniaMetrica> value) { metrica = value; }
    void setUmbral(std::optional<int> value) { umbral = value; }

    // Solo un admin otorga a mano, y solo las de rol: dar "50 encuentros" a
    // dedo vaciaría de sentido a las automáticas, que el job reotorga sola.
    bool esOtorgableAMano() const { return tipo == InsigniaTipo::Rol; }

    nlohmann::json toJson() const;
};

// Una insignia ya otorgada a alguien. Se serializa plana —los campos del
// catálogo junto a los del otorgamiento— porque el cliente la pinta como una
// sola cosa y no tiene nada que hacer con dos objetos anidados.
class InsigniaOtorgada {
private:
    Insignia insignia;
    std::string otorgada_en;
    std::optional<int> otorgada_por;
    std::optional<std::string> motivo;

public:
    const Insignia& getInsignia() const { return insignia; }
    const std::string& getOtorgadaEn() const { return otorgada_en; }
    const std::optional<int>& getOtorgadaPor() const { return otorgada_por; }
    const std::optional<std::string>& getMotivo() const { return motivo; }

    void setInsignia(Insignia value) { insignia = std::move(value); }
    void setOtorgadaEn(std::string value) { otorgada_en = std::move(value); }
    void setOtorgadaPor(std::optional<int> value) { otorgada_por = value; }
    void setMotivo(std::optional<std::string> value) { motivo = std::move(value); }

    nlohmann::json toJson() const;
};

// Cuerpo de `POST /api/v1/insignias/otorgar`. Quien otorga sale de la
// identidad verificada, nunca del cuerpo.
struct OtorgamientoInsignia {
    int usuarioId = 0;
    std::string codigo;
    std::optional<std::string> motivo;

    static OtorgamientoInsignia fromJson(const nlohmann::json& json);
};

#endif // INSIGNIA_HPP
