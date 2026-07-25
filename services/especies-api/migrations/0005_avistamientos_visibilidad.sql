BEGIN;

-- Avistamientos privados por defecto ("mis encuentros"): un usuario marca
-- una especie como vista con nota/ubicación/foto propia, visible solo para
-- él. Puede luego compartirlo con la comunidad (visibilidad='publico'),
-- momento en que entra a la cola de moderación pública existente
-- (estado pendiente/aprobado/rechazado, sin cambios de esa parte).
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'avistamiento_visibilidad_enum') THEN
        CREATE TYPE avistamiento_visibilidad_enum AS ENUM ('privado', 'publico');
    END IF;
END;
$$;

ALTER TABLE avistamientos
    ADD COLUMN IF NOT EXISTS visibilidad avistamiento_visibilidad_enum NOT NULL DEFAULT 'privado';

CREATE INDEX IF NOT EXISTS idx_avistamientos_visibilidad ON avistamientos (visibilidad);

COMMIT;
