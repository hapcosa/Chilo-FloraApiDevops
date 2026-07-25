BEGIN;

-- Categorías de moderación: unidad de asignación entre moderadores y
-- especies. Puede ser tan amplia como un reino completo (reinos con poca
-- documentación, ej. Fungi) o tan específica como un subgrupo dentro de un
-- reino (ej. "Aves" o "Anfibios" dentro de Animalia). Toda especie
-- pertenece a exactamente una categoría (ver columna en `especies` abajo).
CREATE TABLE IF NOT EXISTS categorias_moderacion (
    id SERIAL PRIMARY KEY,
    reino reino_enum NOT NULL,
    nombre VARCHAR(80) NOT NULL,
    descripcion TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(reino, nombre)
);

ALTER TABLE especies
    ADD COLUMN IF NOT EXISTS categoria_moderacion_id INTEGER
        REFERENCES categorias_moderacion(id) ON DELETE RESTRICT;

CREATE INDEX IF NOT EXISTS idx_especies_categoria_moderacion
    ON especies (categoria_moderacion_id);

-- Asignación muchos-a-muchos: un moderador puede cubrir varias categorías,
-- una categoría puede tener varios moderadores. user_id es una referencia
-- lógica a auth-service (mismo patrón que especies.creado_por): sin FK
-- formal porque la tabla de usuarios vive en otra base de datos.
CREATE TABLE IF NOT EXISTS moderador_categorias (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL,
    categoria_moderacion_id INTEGER NOT NULL
        REFERENCES categorias_moderacion(id) ON DELETE CASCADE,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    UNIQUE(user_id, categoria_moderacion_id)
);

CREATE INDEX IF NOT EXISTS idx_moderador_categorias_user
    ON moderador_categorias (user_id);
CREATE INDEX IF NOT EXISTS idx_moderador_categorias_categoria
    ON moderador_categorias (categoria_moderacion_id);

COMMIT;
