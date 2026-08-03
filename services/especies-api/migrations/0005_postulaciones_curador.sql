-- =============================================================================
-- 0005_postulaciones_curador.sql
-- =============================================================================
-- Cómo se llega a ser curador (docs/PLAN_MAESTRO.md §10, ADR #14).
--
-- La migración 0004 creó `moderador_categorias` pero no dijo quién la llena:
-- hoy solo un admin, a mano por la API. Esta tabla es la vía por la que un
-- usuario cualquiera pide la curaduría de una categoría y un admin resuelve.
--
-- Aprobar inserta la fila en `moderador_categorias` en la **misma
-- transacción** que marca la postulación como aprobada, y **no** toca los
-- roles de la BD del auth-service: un curador sigue siendo rol `user` con
-- asignaciones. Es lo que evita acoplar las dos bases (ADR #14).
--
-- `usuario_id` / `revisado_por` referencian lógicamente a `usuarios`, que vive
-- en la BD del auth-service. Sin FK formal, igual que `especies.creado_por`.
-- =============================================================================

BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'postulacion_estado_enum') THEN
        CREATE TYPE postulacion_estado_enum AS ENUM ('pendiente', 'aprobada', 'rechazada');
    END IF;
END;
$$;

CREATE TABLE IF NOT EXISTS postulaciones_curador (
    id SERIAL PRIMARY KEY,
    usuario_id INTEGER NOT NULL,
    categoria_id INTEGER NOT NULL
        REFERENCES categorias_moderacion(id) ON DELETE CASCADE,
    texto TEXT NOT NULL,
    estado postulacion_estado_enum NOT NULL DEFAULT 'pendiente',
    revisado_por INTEGER,
    revisado_en TIMESTAMPTZ,
    motivo TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    -- El texto es la única evidencia que el admin tiene para decidir; una
    -- postulación vacía no es una postulación.
    CONSTRAINT postulaciones_curador_texto_not_blank
        CHECK (length(trim(texto)) > 0),
    -- Rechazar sin motivo deja al postulante sin saber qué corregir.
    CONSTRAINT postulaciones_curador_motivo_rechazo
        CHECK (estado <> 'rechazada' OR motivo IS NOT NULL),
    -- Una postulación resuelta registra quién y cuándo; una pendiente, no.
    CONSTRAINT postulaciones_curador_revision_coherente
        CHECK (
            (estado = 'pendiente' AND revisado_por IS NULL AND revisado_en IS NULL)
            OR (estado <> 'pendiente' AND revisado_por IS NOT NULL AND revisado_en IS NOT NULL)
        )
);

-- Único parcial: se puede volver a postular a la misma categoría tras un
-- rechazo (la gente aprende y reintenta), pero no acumular pendientes que
-- dupliquen la bandeja del admin.
CREATE UNIQUE INDEX IF NOT EXISTS uq_postulaciones_curador_pendiente
    ON postulaciones_curador (usuario_id, categoria_id)
    WHERE estado = 'pendiente';

-- La consulta central es la bandeja del admin: pendientes, más antiguas
-- primero.
CREATE INDEX IF NOT EXISTS idx_postulaciones_curador_estado
    ON postulaciones_curador (estado, created_at);

CREATE INDEX IF NOT EXISTS idx_postulaciones_curador_usuario
    ON postulaciones_curador (usuario_id);

DROP TRIGGER IF EXISTS trg_postulaciones_curador_updated_at ON postulaciones_curador;
CREATE TRIGGER trg_postulaciones_curador_updated_at
    BEFORE UPDATE ON postulaciones_curador
    FOR EACH ROW
    EXECUTE FUNCTION set_updated_at();

COMMIT;
