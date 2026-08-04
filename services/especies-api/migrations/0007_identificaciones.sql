-- =============================================================================
-- 0007_identificaciones.sql
-- =============================================================================
-- Identificación comunitaria de avistamientos (docs/PLAN_MAESTRO.md §10, ADR #14).
--
-- Hasta aquí, decir qué especie es un avistamiento era potestad de un
-- moderador y ocurría de hecho en `avistamientos.especie_id`, sin registro de
-- quién lo propuso ni de si alguien discrepaba. Esta migración separa las dos
-- cosas:
--
--   - `avistamiento_identificaciones`: quién dice que es qué. Es la evidencia.
--   - `avistamientos.grado_identificacion`: la conclusión derivada de esa
--     evidencia.
--
-- `estado` (`pendiente/aprobado/rechazado`) NO se toca: sigue siendo moderación
-- de contenido —ocultar una foto inapropiada— y es independiente del grado. Un
-- avistamiento puede estar aprobado y sin identificar, o en discusión y
-- rechazado.
--
-- El grado se recalcula en la capa de servicio (`IdentificacionService`), no en
-- un trigger. Es deliberado: la regla de quórum va a cambiar cuando la
-- comunidad crezca, y las migraciones no se editan una vez mergeadas. Un
-- trigger obligaría a una migración nueva por cada ajuste de la regla.
--
-- `usuario_id` referencia lógicamente a `usuarios`, que vive en la BD del
-- auth-service. Sin FK formal, igual que `especies.creado_por`.
-- =============================================================================

BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'grado_identificacion_enum') THEN
        CREATE TYPE grado_identificacion_enum AS ENUM (
            'sin_identificar',  -- nadie propuso nada todavía
            'en_discusion',     -- hay propuestas pero no hay acuerdo suficiente
            'investigacion'     -- hay acuerdo: sirve como dato
        );
    END IF;
END;
$$;

CREATE TABLE IF NOT EXISTS avistamiento_identificaciones (
    id SERIAL PRIMARY KEY,
    avistamiento_id INTEGER NOT NULL
        REFERENCES avistamientos(id) ON DELETE CASCADE,
    usuario_id INTEGER NOT NULL,
    -- La identificación apunta a una ficha del catálogo, no a texto libre: es
    -- lo que permite contar coincidencias sin normalizar nombres a mano.
    especie_id INTEGER NOT NULL
        REFERENCES especies(id) ON DELETE CASCADE,
    comentario TEXT,
    -- Voto decisivo: se marca en el momento de crearla si quien identifica cura
    -- la categoría de esa especie. Se persiste en vez de recalcularse porque el
    -- grado tiene que poder recomputarse a partir de estas filas y nada más;
    -- si mañana se le quita la categoría a un curador, las decisiones que ya
    -- tomó no se reescriben solas.
    decisiva BOOLEAN NOT NULL DEFAULT false,
    -- Retirar en vez de borrar: cambiar de opinión es parte del proceso y el
    -- historial explica cómo se llegó al grado actual.
    retirada BOOLEAN NOT NULL DEFAULT false,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Una persona, un voto vigente. Parcial sobre `retirada` para que se pueda
-- retirar una identificación y proponer otra sin chocar con el índice.
CREATE UNIQUE INDEX IF NOT EXISTS uq_avistamiento_identificaciones_vigente
    ON avistamiento_identificaciones (avistamiento_id, usuario_id)
    WHERE NOT retirada;

-- La consulta central: todas las identificaciones vigentes de un avistamiento,
-- que es lo que se cuenta para recalcular el grado.
CREATE INDEX IF NOT EXISTS idx_avistamiento_identificaciones_avistamiento
    ON avistamiento_identificaciones (avistamiento_id)
    WHERE NOT retirada;

CREATE INDEX IF NOT EXISTS idx_avistamiento_identificaciones_usuario
    ON avistamiento_identificaciones (usuario_id);

DROP TRIGGER IF EXISTS trg_avistamiento_identificaciones_updated_at
    ON avistamiento_identificaciones;
CREATE TRIGGER trg_avistamiento_identificaciones_updated_at
    BEFORE UPDATE ON avistamiento_identificaciones
    FOR EACH ROW
    EXECUTE FUNCTION set_updated_at();

-- El DEFAULT vale también como backfill de las filas existentes: ninguna tiene
-- identificaciones todavía, así que 'sin_identificar' es literalmente cierto.
-- Los avistamientos que ya traen `especie_id` puesto a mano por un moderador
-- quedan igualmente en 'sin_identificar': no hay evidencia registrada que
-- sostenga otro grado, y esta migración no inventa la que falta.
ALTER TABLE avistamientos
    ADD COLUMN IF NOT EXISTS grado_identificacion grado_identificacion_enum
        NOT NULL DEFAULT 'sin_identificar';

CREATE INDEX IF NOT EXISTS idx_avistamientos_grado_identificacion
    ON avistamientos (grado_identificacion);

COMMIT;
