-- =============================================================================
-- 0008_avistamientos_visibilidad.sql
-- =============================================================================
-- Encuentros privados por defecto (docs/PLAN_MAESTRO.md §10, ADR #12).
--
-- La Fase 6 hizo los avistamientos públicos por construcción: no había forma de
-- registrar uno sin ofrecerlo al catálogo. El ADR #12 decidió lo contrario —un
-- avistamiento nace privado y su dueño lo comparte a mano— pero la columna
-- nunca se creó, así que hasta ahora "mis encuentros" y "avistamientos de la
-- comunidad" eran literalmente lo mismo. Sin esta migración, el feed comunitario
-- publicaría encuentros que su autor cree privados.
--
-- `visibilidad` es independiente de `estado`: el primero lo decide el autor, el
-- segundo la moderación. Un encuentro privado no pasa por moderación porque
-- nunca se ofreció a nadie; compartirlo es lo que lo pone en la cola.
--
-- Backfill: las filas ya aprobadas por un moderador pasan a 'publico'. Bajo las
-- reglas de la Fase 6 se registraron para el catálogo público y un moderador las
-- revisó como tales, así que marcarlas privadas ahora rompería lo que sus
-- autores pidieron. El resto —pendientes y rechazadas— queda privado, que es lo
-- que el ADR #12 quiere para cualquier cosa cuyo destino no esté decidido.
-- =============================================================================

BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'avistamiento_visibilidad_enum') THEN
        CREATE TYPE avistamiento_visibilidad_enum AS ENUM ('privado', 'publico');
    END IF;

    -- El backfill va atado a la creación de la columna: si esto se reaplicara
    -- sobre una base ya migrada, publicaría encuentros que su autor eligió
    -- dejar privados.
    IF NOT EXISTS (SELECT 1 FROM information_schema.columns
                   WHERE table_name = 'avistamientos' AND column_name = 'visibilidad') THEN
        ALTER TABLE avistamientos
            ADD COLUMN visibilidad avistamiento_visibilidad_enum
                NOT NULL DEFAULT 'privado';

        UPDATE avistamientos
            SET visibilidad = 'publico'
            WHERE estado = 'aprobado';
    END IF;
END;
$$;

-- La consulta del feed comunitario: lo público y aprobado, más reciente primero.
-- Parcial porque el feed nunca pide otra cosa, y así el índice no carga con los
-- encuentros privados, que son la mayoría esperada.
CREATE INDEX IF NOT EXISTS idx_avistamientos_feed
    ON avistamientos (observado_en DESC, id DESC)
    WHERE estado = 'aprobado' AND visibilidad = 'publico';

COMMIT;
