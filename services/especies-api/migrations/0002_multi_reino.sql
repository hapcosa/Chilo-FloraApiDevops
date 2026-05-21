-- =============================================================================
-- 0002_multi_reino.sql
-- =============================================================================
-- Evoluciona el schema a multi-reino. Implementa el modelo definido en
-- docs/PLAN_MAESTRO.md §3:
--
--   * Tipo enum `reino_enum` con los 5 reinos acordados.
--   * Tabla base `especies` con columnas comunes + `atributos_especificos JSONB`
--     validado por JSON Schema según el reino (validación en C++, no en SQL).
--   * Foto de portada + array JSONB de claves de object storage, en la propia
--     tabla `especies`. La tabla legacy `especies_imagenes` se mantiene por
--     compatibilidad y se deprecará en un PR posterior con backfill.
--   * Familias unificadas por (reino, nombre) en vez de nombre único global.
--   * Géneros unificados por (familia_id, nombre) en vez de nombre único global.
--   * Campos de divulgación (autor_cientifico, distribucion_chiloe, fuentes,
--     geo_lat/lng).
--   * Auditoría: creado_por, revisado_por, fecha_revision, created_at, updated_at.
--
-- Notas operativas:
--   * Asume BD vacía o sólo con datos del reino "plantae" (estado actual).
--     Backfill: filas existentes en `familias`, `generos`, `especies` reciben
--     reino = 'plantae'. Si en un futuro hay datos de otros reinos cargados
--     antes de esta migración, el DEFAULT debe ajustarse.
--   * Cambios de constraints en este archivo NO usan CONCURRENTLY porque la
--     tabla aún no tiene volumen de producción. Cuando lo tenga, las
--     migraciones futuras deberán evaluarlo.
-- =============================================================================

BEGIN;

-- -----------------------------------------------------------------------------
-- Extensiones requeridas
-- -----------------------------------------------------------------------------
-- pg_trgm: índice GIN para búsqueda fuzzy por nombre común.
CREATE EXTENSION IF NOT EXISTS pg_trgm;

-- -----------------------------------------------------------------------------
-- Tipo enum reino_enum
-- -----------------------------------------------------------------------------
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'reino_enum') THEN
        CREATE TYPE reino_enum AS ENUM (
            'animalia', 'plantae', 'fungi', 'protista', 'monera'
        );
    END IF;
END
$$;

-- -----------------------------------------------------------------------------
-- familias: añadir reino, cambiar UNIQUE, ampliar nombre, created_at
-- -----------------------------------------------------------------------------
ALTER TABLE familias
    ADD COLUMN IF NOT EXISTS reino reino_enum NOT NULL DEFAULT 'plantae';

-- Una vez backfilled, quitamos el DEFAULT para que los inserts futuros sean
-- explícitos sobre a qué reino pertenece la familia.
ALTER TABLE familias
    ALTER COLUMN reino DROP DEFAULT;

ALTER TABLE familias
    ALTER COLUMN nombre TYPE VARCHAR(150);

ALTER TABLE familias
    ADD COLUMN IF NOT EXISTS created_at TIMESTAMPTZ NOT NULL DEFAULT NOW();

-- Reemplazar UNIQUE(nombre) por UNIQUE(reino, nombre).
ALTER TABLE familias DROP CONSTRAINT IF EXISTS familias_nombre_key;
ALTER TABLE familias
    ADD CONSTRAINT familias_reino_nombre_key UNIQUE (reino, nombre);

-- -----------------------------------------------------------------------------
-- generos: UNIQUE compuesto, FK NOT NULL con ON DELETE RESTRICT, created_at
-- -----------------------------------------------------------------------------
ALTER TABLE generos
    ALTER COLUMN nombre TYPE VARCHAR(150);

ALTER TABLE generos
    ALTER COLUMN familia_id SET NOT NULL;

ALTER TABLE generos
    ADD COLUMN IF NOT EXISTS created_at TIMESTAMPTZ NOT NULL DEFAULT NOW();

-- Reemplazar UNIQUE(nombre) por UNIQUE(familia_id, nombre): el mismo nombre
-- de género puede existir en distintas familias.
ALTER TABLE generos DROP CONSTRAINT IF EXISTS generos_nombre_key;
ALTER TABLE generos
    ADD CONSTRAINT generos_familia_nombre_key UNIQUE (familia_id, nombre);

-- Endurecer la FK a ON DELETE RESTRICT (impide borrar una familia con géneros).
ALTER TABLE generos DROP CONSTRAINT IF EXISTS generos_familia_id_fkey;
ALTER TABLE generos
    ADD CONSTRAINT generos_familia_id_fkey
    FOREIGN KEY (familia_id) REFERENCES familias(id) ON DELETE RESTRICT;

-- -----------------------------------------------------------------------------
-- especies: la tabla central. Cambios extensos.
-- -----------------------------------------------------------------------------
-- Reino: por ahora todo lo existente es plantae.
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS reino reino_enum NOT NULL DEFAULT 'plantae';
ALTER TABLE especies
    ALTER COLUMN reino DROP DEFAULT;

-- Ampliar campos de texto.
ALTER TABLE especies
    ALTER COLUMN nombre_cientifico TYPE VARCHAR(200);
ALTER TABLE especies
    ALTER COLUMN nombre_comun TYPE VARCHAR(200);
ALTER TABLE especies
    ALTER COLUMN estado_conservacion TYPE VARCHAR(60);

-- Identificación y contenido divulgativo (nuevas columnas).
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS autor_cientifico VARCHAR(200);

-- distribucion → distribucion_chiloe (renombrado por especificidad geográfica).
ALTER TABLE especies
    RENAME COLUMN distribucion TO distribucion_chiloe;

-- Fuentes bibliográficas / web: array JSONB de objetos {titulo, url, autor, año}.
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS fuentes JSONB NOT NULL DEFAULT '[]'::jsonb;

-- Geolocalización: centroide representativo de avistamientos. Detalle por
-- avistamiento individual irá en la futura tabla `avistamientos` (Fase 6).
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS geo_lat NUMERIC(9, 6);
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS geo_lng NUMERIC(9, 6);

-- Atributos específicos del reino. Validados por JSON Schema en la capa C++.
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS atributos_especificos JSONB NOT NULL DEFAULT '{}'::jsonb;

-- Fotos: claves de object storage (MinIO/S3). NUNCA bytes en la BD.
-- foto_portada_key es la clave de la foto principal; fotos_keys es un array
-- JSONB con el resto. La tabla legacy `especies_imagenes` queda intacta y se
-- migrará/deprecará en un PR aparte con backfill de URLs → keys.
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS foto_portada_key VARCHAR(500);
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS fotos_keys JSONB NOT NULL DEFAULT '[]'::jsonb;

-- Auditoría: creado_por hace referencia lógica al usuario del auth-service.
-- Sin FK formal porque la tabla `usuarios` vive en otra DB lógica (mismo
-- cluster pero distinto servicio dueño). Se valida en aplicación.
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS creado_por INTEGER;

-- Revisión curatorial: especialmente relevante para Fungi (toxicidad).
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS revisado_por INTEGER;
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS fecha_revision TIMESTAMPTZ;

ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS created_at TIMESTAMPTZ NOT NULL DEFAULT NOW();
ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW();

-- Endurecer FK genero_id: NOT NULL + ON DELETE RESTRICT.
ALTER TABLE especies
    ALTER COLUMN genero_id SET NOT NULL;
ALTER TABLE especies DROP CONSTRAINT IF EXISTS especies_genero_id_fkey;
ALTER TABLE especies
    ADD CONSTRAINT especies_genero_id_fkey
    FOREIGN KEY (genero_id) REFERENCES generos(id) ON DELETE RESTRICT;

-- -----------------------------------------------------------------------------
-- Índices
-- -----------------------------------------------------------------------------
CREATE INDEX IF NOT EXISTS idx_especies_reino
    ON especies (reino);

CREATE INDEX IF NOT EXISTS idx_especies_genero_id
    ON especies (genero_id);

-- GIN sobre el JSONB para queries del tipo
--   WHERE atributos_especificos @> '{"comestibilidad":"tóxico"}'
CREATE INDEX IF NOT EXISTS idx_especies_atributos
    ON especies USING GIN (atributos_especificos);

-- Búsqueda fuzzy por nombre común usando trigramas.
CREATE INDEX IF NOT EXISTS idx_especies_nombre_comun_trgm
    ON especies USING GIN (nombre_comun gin_trgm_ops);

-- -----------------------------------------------------------------------------
-- Trigger para mantener updated_at
-- -----------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION set_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_especies_updated_at ON especies;
CREATE TRIGGER trg_especies_updated_at
    BEFORE UPDATE ON especies
    FOR EACH ROW
    EXECUTE FUNCTION set_updated_at();

COMMIT;
