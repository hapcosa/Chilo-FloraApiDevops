-- =============================================================================
-- 0014_insignias.sql
-- =============================================================================
-- Insignias de la Fase 9 (docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md, PR 11).
--
-- Dos familias en la misma tabla, distinguidas por `tipo`:
--
--   * `automatica`: se derivan de la actividad (encuentros, especies
--     distintas, reinos cubiertos, encuentros que otros identificaron). El
--     criterio se guarda como datos —`metrica` + `umbral`— y no como código,
--     así que el recálculo es UNA consulta que sirve para todo el catálogo y
--     añadir una insignia nueva es una fila, no un deploy.
--   * `rol`: las otorga un admin a mano desde el panel (moderador, curador,
--     administrador). No tienen umbral porque no se ganan contando nada.
--
-- Se recalculan en un job, nunca al vuelo: contar encuentros en cada request
-- del feed sería un escaneo por fila mostrada. El disparador es
-- `POST /api/v1/insignias/recalcular` (solo admin, idempotente), que el panel
-- expone como botón y el host puede meter en un cron.
--
-- **No hay ranking público** ni tabla de posiciones, y es deliberado: premiar
-- el volumen empuja exactamente la conducta que la Fase 9.0 desalienta
-- (perseguir bichos para sumar). Las insignias se ven en el perfil propio, en
-- el perfil público y junto al nombre en el feed. Nada más.
--
-- `usuario_id` / `otorgada_por` referencian lógicamente a `usuarios`, que vive
-- en la BD del auth-service. Sin FK formal, igual que `especies.creado_por`.
-- =============================================================================

BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'insignia_tipo_enum') THEN
        CREATE TYPE insignia_tipo_enum AS ENUM ('automatica', 'rol');
    END IF;
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'insignia_metrica_enum') THEN
        CREATE TYPE insignia_metrica_enum AS ENUM (
            'encuentros',
            'especies_distintas',
            'reinos',
            'identificado_por_otros'
        );
    END IF;
END;
$$;

CREATE TABLE IF NOT EXISTS insignias (
    id SERIAL PRIMARY KEY,
    -- El código es la identidad estable: la app elige el ícono por él y el
    -- panel otorga las de rol por código, no por id.
    codigo VARCHAR(60) NOT NULL UNIQUE,
    nombre VARCHAR(120) NOT NULL,
    descripcion TEXT NOT NULL,
    -- Texto que se le muestra a la persona ("10 encuentros aprobados"). Es
    -- también el motivo con el que el recálculo otorga las automáticas.
    criterio TEXT NOT NULL,
    tipo insignia_tipo_enum NOT NULL,
    metrica insignia_metrica_enum,
    umbral INTEGER,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT insignias_codigo_formato
        CHECK (codigo ~ '^[a-z0-9]+(-[a-z0-9]+)*$'),
    CONSTRAINT insignias_nombre_not_blank
        CHECK (length(trim(nombre)) > 0),
    -- Una automática sin métrica y umbral no la otorgaría nunca el job; una de
    -- rol con umbral fingiría un criterio que nadie evalúa.
    CONSTRAINT insignias_criterio_coherente
        CHECK (
            (tipo = 'automatica' AND metrica IS NOT NULL AND umbral IS NOT NULL AND umbral > 0)
            OR (tipo = 'rol' AND metrica IS NULL AND umbral IS NULL)
        )
);

-- El recálculo recorre el catálogo automático por métrica.
CREATE INDEX IF NOT EXISTS idx_insignias_tipo_metrica
    ON insignias (tipo, metrica);

DROP TRIGGER IF EXISTS trg_insignias_updated_at ON insignias;
CREATE TRIGGER trg_insignias_updated_at
    BEFORE UPDATE ON insignias
    FOR EACH ROW
    EXECUTE FUNCTION set_updated_at();

-- Una insignia por persona: la PK compuesta es lo que hace idempotente al
-- recálculo (INSERT ... ON CONFLICT DO NOTHING).
CREATE TABLE IF NOT EXISTS usuario_insignias (
    usuario_id INTEGER NOT NULL,
    insignia_id INTEGER NOT NULL
        REFERENCES insignias(id) ON DELETE CASCADE,
    otorgada_en TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    -- NULL cuando la otorgó el job; el id del admin cuando fue a mano.
    otorgada_por INTEGER,
    motivo TEXT,

    PRIMARY KEY (usuario_id, insignia_id)
);

-- Sentido inverso ("¿quiénes tienen esta insignia?"), para el panel.
CREATE INDEX IF NOT EXISTS idx_usuario_insignias_insignia
    ON usuario_insignias (insignia_id);

-- Catálogo inicial. ON CONFLICT para que reaplicar la migración sobre una BD
-- ya poblada no falle ni pise ediciones hechas después.
INSERT INTO insignias (codigo, nombre, descripcion, criterio, tipo, metrica, umbral) VALUES
    ('primer-encuentro', 'Primer encuentro',
     'Compartiste tu primer encuentro con la comunidad.',
     '1 encuentro aprobado', 'automatica', 'encuentros', 1),
    ('observador', 'Observador',
     'Ya llevas una decena de encuentros documentados.',
     '10 encuentros aprobados', 'automatica', 'encuentros', 10),
    ('constante', 'Constante',
     'Cincuenta encuentros: mirar seguido es la mitad del oficio.',
     '50 encuentros aprobados', 'automatica', 'encuentros', 50),
    ('curioso', 'Curioso',
     'Diez especies distintas en tus encuentros.',
     '10 especies distintas', 'automatica', 'especies_distintas', 10),
    ('coleccionista', 'Coleccionista',
     'Treinta especies distintas en tus encuentros.',
     '30 especies distintas', 'automatica', 'especies_distintas', 30),
    ('tres-reinos', 'Tres reinos',
     'Tus encuentros cubren tres de los cinco reinos.',
     '3 reinos distintos', 'automatica', 'reinos', 3),
    ('cinco-reinos', 'Cinco reinos',
     'Tus encuentros cubren los cinco reinos de Chiloé.',
     '5 reinos distintos', 'automatica', 'reinos', 5),
    ('en-comunidad', 'En comunidad',
     'Otras personas identificaron cinco de tus encuentros.',
     '5 encuentros identificados por otros', 'automatica', 'identificado_por_otros', 5),
    ('moderador', 'Moderador',
     'Revisa encuentros y fichas de todo el catálogo.',
     'Otorgada por un administrador', 'rol', NULL, NULL),
    ('curador', 'Curador',
     'Cura una categoría del catálogo. El motivo indica cuál.',
     'Otorgada por un administrador', 'rol', NULL, NULL),
    ('administrador', 'Administrador',
     'Administra la plataforma.',
     'Otorgada por un administrador', 'rol', NULL, NULL)
ON CONFLICT (codigo) DO NOTHING;

COMMIT;
