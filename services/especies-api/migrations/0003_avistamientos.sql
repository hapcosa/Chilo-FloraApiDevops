BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'avistamiento_estado_enum') THEN
        CREATE TYPE avistamiento_estado_enum AS ENUM ('pendiente', 'aprobado', 'rechazado');
    END IF;
END;
$$;

CREATE TABLE IF NOT EXISTS avistamientos (
    id SERIAL PRIMARY KEY,
    especie_id INTEGER REFERENCES especies(id) ON DELETE SET NULL,
    reino reino_enum NOT NULL,
    nombre_sugerido VARCHAR(200),
    descripcion TEXT,
    foto_key VARCHAR(500) NOT NULL,
    geo_lat NUMERIC(9, 6) NOT NULL,
    geo_lng NUMERIC(9, 6) NOT NULL,
    precision_metros NUMERIC(8, 2),
    observado_en TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    creado_por INTEGER,
    estado avistamiento_estado_enum NOT NULL DEFAULT 'pendiente',
    moderado_por INTEGER,
    moderado_en TIMESTAMPTZ,
    motivo_rechazo TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT avistamientos_foto_key_not_blank
        CHECK (length(trim(foto_key)) > 0),
    CONSTRAINT avistamientos_lat_range
        CHECK (geo_lat >= -90 AND geo_lat <= 90),
    CONSTRAINT avistamientos_lng_range
        CHECK (geo_lng >= -180 AND geo_lng <= 180),
    CONSTRAINT avistamientos_precision_non_negative
        CHECK (precision_metros IS NULL OR precision_metros >= 0),
    CONSTRAINT avistamientos_moderacion_rechazo
        CHECK (estado <> 'rechazado' OR motivo_rechazo IS NOT NULL)
);

CREATE INDEX IF NOT EXISTS idx_avistamientos_estado
    ON avistamientos (estado);

CREATE INDEX IF NOT EXISTS idx_avistamientos_reino
    ON avistamientos (reino);

CREATE INDEX IF NOT EXISTS idx_avistamientos_especie_id
    ON avistamientos (especie_id);

CREATE INDEX IF NOT EXISTS idx_avistamientos_creado_por
    ON avistamientos (creado_por);

CREATE INDEX IF NOT EXISTS idx_avistamientos_observado_en
    ON avistamientos (observado_en DESC);

DROP TRIGGER IF EXISTS trg_avistamientos_updated_at ON avistamientos;
CREATE TRIGGER trg_avistamientos_updated_at
    BEFORE UPDATE ON avistamientos
    FOR EACH ROW
    EXECUTE FUNCTION set_updated_at();

COMMIT;

