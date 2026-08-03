-- =============================================================================
-- 0004_categorias_moderacion.sql
-- =============================================================================
-- Moderación por categoría (docs/PLAN_MAESTRO.md §10, ADR #11 y ADR #14).
--
-- Hasta ahora cualquier usuario con rol `admin` o `moderator` podía editar
-- cualquier especie de cualquier reino. Esta migración introduce el eje sobre
-- el que se restringe ese permiso:
--
--   * `categorias_moderacion`: subgrupo curable dentro de un reino (p. ej.
--     "Aves" dentro de animalia). Sin jerarquía: el ADR #11 no la pide.
--   * `moderador_categorias`: relación muchos a muchos usuario ↔ categoría.
--     Un curador puede cubrir varias categorías y una categoría puede tener
--     varios curadores.
--   * `especies.categoria_id`: a qué categoría pertenece cada ficha.
--
-- `usuario_id` / `asignado_por` referencian lógicamente a `usuarios`, que vive
-- en la BD del auth-service. Sin FK formal, igual que `especies.creado_por`;
-- se valida en aplicación.
--
-- `categoria_id` queda NULLABLE porque ya hay especies en producción; el
-- backfill de más abajo las clasifica y endurecer a NOT NULL se hará en una
-- migración posterior, cuando ningún entorno tenga huérfanas.
--
-- Las cinco categorías "general" y el backfill van aquí y no en `seeds/` pese
-- a ser filas: no son contenido divulgativo sino el mínimo de referencia que
-- hace usable la columna, y deben existir idénticas en todos los entornos —
-- que es justo lo que garantiza una migración y no un seed (los seeds se
-- aplican a mano). Las subcategorías reales ("Aves", "Hongos") las crean los
-- admins por la API.
-- =============================================================================

BEGIN;

CREATE TABLE IF NOT EXISTS categorias_moderacion (
    id SERIAL PRIMARY KEY,
    slug VARCHAR(60) NOT NULL UNIQUE,
    nombre VARCHAR(120) NOT NULL,
    reino reino_enum NOT NULL,
    descripcion TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT categorias_moderacion_slug_formato
        CHECK (slug ~ '^[a-z0-9]+(-[a-z0-9]+)*$'),
    CONSTRAINT categorias_moderacion_nombre_not_blank
        CHECK (length(trim(nombre)) > 0)
);

CREATE INDEX IF NOT EXISTS idx_categorias_moderacion_reino
    ON categorias_moderacion (reino);

DROP TRIGGER IF EXISTS trg_categorias_moderacion_updated_at ON categorias_moderacion;
CREATE TRIGGER trg_categorias_moderacion_updated_at
    BEFORE UPDATE ON categorias_moderacion
    FOR EACH ROW
    EXECUTE FUNCTION set_updated_at();

CREATE TABLE IF NOT EXISTS moderador_categorias (
    usuario_id INTEGER NOT NULL,
    categoria_id INTEGER NOT NULL
        REFERENCES categorias_moderacion(id) ON DELETE CASCADE,
    asignado_por INTEGER,
    asignado_en TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    PRIMARY KEY (usuario_id, categoria_id)
);

-- El chequeo de permiso pregunta siempre "¿qué categorías cubre este
-- usuario?", así que la PK compuesta ya sirve. Este índice cubre el sentido
-- inverso ("¿quiénes curan esta categoría?"), usado por el panel de admin.
CREATE INDEX IF NOT EXISTS idx_moderador_categorias_categoria
    ON moderador_categorias (categoria_id);

ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS categoria_id INTEGER
        REFERENCES categorias_moderacion(id) ON DELETE RESTRICT;

CREATE INDEX IF NOT EXISTS idx_especies_categoria_id
    ON especies (categoria_id);

-- -----------------------------------------------------------------------------
-- Categoría "general" por reino + clasificación de lo ya existente
-- -----------------------------------------------------------------------------
INSERT INTO categorias_moderacion (slug, nombre, reino, descripcion) VALUES
    ('animalia-general', 'Animalia (general)', 'animalia',
     'Todo lo que aún no tiene una categoría más específica dentro del reino.'),
    ('plantae-general',  'Plantae (general)',  'plantae',
     'Todo lo que aún no tiene una categoría más específica dentro del reino.'),
    ('fungi-general',    'Fungi (general)',    'fungi',
     'Todo lo que aún no tiene una categoría más específica dentro del reino.'),
    ('protista-general', 'Protista (general)', 'protista',
     'Todo lo que aún no tiene una categoría más específica dentro del reino.'),
    ('monera-general',   'Monera (general)',   'monera',
     'Todo lo que aún no tiene una categoría más específica dentro del reino.')
ON CONFLICT (slug) DO NOTHING;

UPDATE especies e
SET categoria_id = c.id
FROM categorias_moderacion c
WHERE e.categoria_id IS NULL
  AND c.reino = e.reino
  AND c.slug = e.reino::text || '-general';

COMMIT;
