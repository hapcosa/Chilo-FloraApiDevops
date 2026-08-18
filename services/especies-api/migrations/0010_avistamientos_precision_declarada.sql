-- =============================================================================
-- 0010_avistamientos_precision_declarada.sql
-- =============================================================================
-- Encuentros anteriores a la app (Fase 9, PR 7).
--
-- Hasta ahora un encuentro se registraba siempre "aquí y ahora": el cliente
-- mandaba la lectura del GPS y `observado_en` era el instante de guardarlo. Al
-- permitir registrar un recuerdo de hace tres años, la ubicación deja de venir
-- de un sensor y pasa a venir de la memoria de alguien. Guardar eso con la
-- misma confianza que una lectura de 5 m contamina el mapa: un punto caliente
-- de la Fase 9.2 podría estar formado por recuerdos aproximados.
--
-- `precision_metros` no sirve para expresarlo. Es una medición del sensor, y un
-- recuerdo no tiene una: inventarle un número sería peor que no tenerlo.
-- `precision_declarada` es lo que la persona afirma sobre su propio dato.
--
-- Backfill implícito por el DEFAULT: todas las filas existentes se registraron
-- con GPS en vivo desde la app, así que 'exacto' es lo que efectivamente son.
-- =============================================================================

BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'precision_declarada_enum') THEN
        CREATE TYPE precision_declarada_enum AS ENUM ('exacto', 'aproximado', 'zona');
    END IF;
END;
$$;

ALTER TABLE avistamientos
    ADD COLUMN IF NOT EXISTS precision_declarada precision_declarada_enum
        NOT NULL DEFAULT 'exacto';

COMMIT;
