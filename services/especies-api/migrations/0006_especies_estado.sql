-- =============================================================================
-- 0006_especies_estado.sql
-- =============================================================================
-- Borrador / publicada para las fichas de especie (docs/PLAN_MAESTRO.md §10,
-- ADR #14).
--
-- Hasta ahora toda ficha creada era inmediatamente visible. Un curador que
-- empieza una ficha y la deja a medias la publica sin querer: el cache SQLite
-- del móvil se llena desde `GET /especies`, así que lo que entra en la lista
-- termina en los teléfonos.
--
-- Las fichas existentes se quedan en 'publicada': ya estaban visibles y
-- esconderlas ahora sería una regresión para las apps ya instaladas. Por eso
-- el DEFAULT de la columna es 'publicada' aunque la API cree las nuevas como
-- borrador — el default cubre el backfill, no la política de la API.
--
-- `publicado_por` referencia lógicamente a `usuarios`, que vive en la BD del
-- auth-service. Sin FK formal, igual que `especies.creado_por`.
-- =============================================================================

BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'especie_estado_enum') THEN
        CREATE TYPE especie_estado_enum AS ENUM ('borrador', 'publicada');
    END IF;
END;
$$;

ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS estado especie_estado_enum NOT NULL DEFAULT 'publicada',
    ADD COLUMN IF NOT EXISTS publicado_por INTEGER,
    ADD COLUMN IF NOT EXISTS fecha_publicacion TIMESTAMPTZ;

-- Un borrador no puede arrastrar la firma de una publicación anterior: al
-- despublicar se limpian ambas. En sentido contrario no se exige nada porque
-- las fichas anteriores a esta migración están publicadas sin saber por quién.
ALTER TABLE especies
    DROP CONSTRAINT IF EXISTS especies_borrador_sin_publicacion;
ALTER TABLE especies
    ADD CONSTRAINT especies_borrador_sin_publicacion
    CHECK (estado <> 'borrador'
           OR (publicado_por IS NULL AND fecha_publicacion IS NULL));

-- El listado público filtra siempre por estado; el índice compuesto con
-- categoria_id cubre además la vista del curador (sus borradores).
CREATE INDEX IF NOT EXISTS idx_especies_estado
    ON especies (estado);
CREATE INDEX IF NOT EXISTS idx_especies_estado_categoria
    ON especies (estado, categoria_id);

COMMIT;
