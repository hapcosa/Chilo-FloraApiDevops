-- =============================================================================
-- 0012_areas_protegidas.sql
-- =============================================================================
-- Parques y áreas protegidas de Chiloé (Fase 9, PR 10).
--
-- El gancho para el visitante no es "dónde está el bicho" sino "a qué parque
-- voy y qué puedo ver ahí". Eso pide una entidad propia: un área protegida
-- tiene administrador, accesos y sitio web, cosas que no son una etiqueta de un
-- avistamiento.
--
-- Sobre la geometría: este proyecto no tiene PostGIS y no lo introduce por
-- esto (ADR #23). El polígono real de cada área se guarda como **GeoJSON en
-- `geometria` (JSONB)**, que es lo que el cliente necesita para dibujarlo y no
-- requiere extensión ninguna; lo que el servidor consulta —qué áreas caen en el
-- mapa, qué especies se han visto dentro— sale del **bounding box** en columnas
-- NUMERIC indexadas. El bbox es una aproximación del área: incluye de más en
-- costas recortadas como la de Chiloé. Se asume a sabiendas, porque la
-- alternativa es una dependencia de infraestructura por una consulta.
--
-- `geometria` nace NULL en todo el seed: no tenemos los polígonos oficiales y
-- dibujar uno inventado sería peor que no dibujar nada — parecería el límite
-- real del parque. Los carga curaduría desde las fuentes de CONAF y de cada
-- administrador, con su licencia anotada en `fuente`.
--
-- Por lo mismo, `verificado` arranca en `false` para todas: los datos del seed
-- son de conocimiento público y coordenadas aproximadas, no una fuente
-- validada. La app debe poder decir "dato sin verificar" hasta que curaduría
-- pase por cada ficha.
-- =============================================================================

BEGIN;

DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'area_protegida_tipo_enum') THEN
        CREATE TYPE area_protegida_tipo_enum AS ENUM (
            'parque_nacional',
            'reserva_nacional',
            'monumento_natural',
            'santuario_naturaleza',
            'parque_privado',
            'sitio_ramsar',
            'humedal_urbano'
        );
    END IF;
END;
$$;

CREATE TABLE IF NOT EXISTS areas_protegidas (
    id              SERIAL PRIMARY KEY,
    nombre          VARCHAR(200) NOT NULL UNIQUE,
    tipo            area_protegida_tipo_enum NOT NULL,
    descripcion     TEXT,
    administrador   VARCHAR(200),
    accesos         TEXT,
    sitio_web       VARCHAR(300),
    -- Centroide para el pin del mapa y bbox para las consultas espaciales.
    centro_lat      NUMERIC(9, 6) NOT NULL,
    centro_lng      NUMERIC(9, 6) NOT NULL,
    min_lat         NUMERIC(9, 6) NOT NULL,
    min_lng         NUMERIC(9, 6) NOT NULL,
    max_lat         NUMERIC(9, 6) NOT NULL,
    max_lng         NUMERIC(9, 6) NOT NULL,
    -- GeoJSON del polígono real, cuando curaduría lo cargue. Ver cabecera.
    geometria       JSONB,
    superficie_ha   NUMERIC(12, 2),
    fuente          TEXT,
    verificado      BOOLEAN NOT NULL DEFAULT FALSE,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at      TIMESTAMPTZ NOT NULL DEFAULT NOW(),

    CONSTRAINT areas_protegidas_bbox_coherente
        CHECK (min_lat <= max_lat AND min_lng <= max_lng),
    CONSTRAINT areas_protegidas_centro_dentro_bbox
        CHECK (centro_lat BETWEEN min_lat AND max_lat
               AND centro_lng BETWEEN min_lng AND max_lng)
);

CREATE INDEX IF NOT EXISTS idx_areas_protegidas_bbox
    ON areas_protegidas (min_lat, max_lat, min_lng, max_lng);
CREATE INDEX IF NOT EXISTS idx_areas_protegidas_tipo
    ON areas_protegidas (tipo);

DROP TRIGGER IF EXISTS trg_areas_protegidas_updated_at ON areas_protegidas;
CREATE TRIGGER trg_areas_protegidas_updated_at
    BEFORE UPDATE ON areas_protegidas
    FOR EACH ROW EXECUTE FUNCTION set_updated_at();

-- -----------------------------------------------------------------------------
-- Seed. Coordenadas aproximadas de conocimiento público; ninguna está
-- verificada contra fuente oficial, de ahí `verificado = false`. Las
-- superficies son las cifras que publican los propios administradores.
-- `ON CONFLICT DO NOTHING` para que reaplicar no pise correcciones de curaduría.
-- -----------------------------------------------------------------------------
INSERT INTO areas_protegidas
    (nombre, tipo, descripcion, administrador, accesos, sitio_web,
     centro_lat, centro_lng, min_lat, min_lng, max_lat, max_lng,
     superficie_ha, fuente)
VALUES
    ('Parque Nacional Chiloé', 'parque_nacional',
     'Bosque siempreverde valdiviano y dunas costeras en la vertiente occidental de la Isla Grande. Sectores Chepu (norte), Anay/Cucao (centro) y Chanquín.',
     'CONAF',
     'Sector Anay por Cucao, desde Castro vía Chonchi. Sector Chepu por el norte, desde Ancud.',
     'https://www.conaf.cl',
     -42.6300, -74.0500, -42.9000, -74.1500, -42.1500, -73.9000,
     43057.00,
     'Conocimiento público; pendiente de validar contra CONAF (límites y superficie).'),

    ('Parque Tantauco', 'parque_privado',
     'Parque privado de conservación en el extremo sur de la Isla Grande, con turberas, bosque siempreverde y avistamiento de fauna nativa.',
     'Fundación Tantauco',
     'Portería norte en Yaldad, cerca de Quellón. Sector sur por Inío, con acceso por mar o aire.',
     'https://www.parquetantauco.cl',
     -43.1500, -74.0000, -43.4500, -74.3000, -42.9500, -73.6000,
     118000.00,
     'Conocimiento público; pendiente de validar contra el administrador del parque.'),

    ('Parque Tepuhueico', 'parque_privado',
     'Parque privado sobre el lago Tepuhueico, bosque nativo costero y hábitat de zorro de Darwin.',
     'Parque Tepuhueico',
     'Camino desde Chonchi hacia el sur, por ruta interior.',
     NULL,
     -42.8500, -73.9500, -42.9500, -74.0500, -42.7500, -73.8500,
     20000.00,
     'Conocimiento público; pendiente de validar contra el administrador del parque.'),

    ('Parque Ahuenco', 'parque_privado',
     'Parque privado de conservación en la costa noroeste de la Isla Grande, sobre bosque siempreverde y litoral abierto al Pacífico.',
     'Corporación Parque Ahuenco',
     'Acceso restringido, por sendero desde el sector de Chepu.',
     NULL,
     -42.0500, -74.0300, -42.1200, -74.0800, -41.9800, -73.9800,
     1000.00,
     'Conocimiento público; pendiente de validar contra el administrador del parque.'),

    ('Monumento Natural Islotes de Puñihuil', 'monumento_natural',
     'Tres islotes frente a la costa norponiente donde anidan juntas la única colonia mixta conocida de pingüino de Humboldt y pingüino de Magallanes.',
     'CONAF',
     'Por Ancud hacia Puñihuil; el avistamiento se hace en bote desde la playa.',
     'https://www.conaf.cl',
     -41.9200, -74.0300, -41.9300, -74.0400, -41.9100, -74.0200,
     8.90,
     'Conocimiento público; pendiente de validar contra CONAF.'),

    ('Humedal de Caulín', 'humedal_urbano',
     'Estuario del norte de la isla, parada de aves playeras migratorias y de cisnes de cuello negro.',
     'Municipalidad de Ancud',
     'Desvío desde la ruta 5 a la altura de Caulín, cerca del cruce a Chacao.',
     NULL,
     -41.8200, -73.6300, -41.8500, -73.6800, -41.7900, -73.5800,
     NULL,
     'Conocimiento público; pendiente de validar el estado de protección y sus límites.'),

    ('Humedales de Putemún', 'santuario_naturaleza',
     'Humedales costeros al norte de Castro, sitio de alimentación de aves playeras migratorias del Pacífico.',
     'Ministerio del Medio Ambiente',
     'Al norte de Castro por la ruta 5, sector Putemún.',
     NULL,
     -42.4200, -73.7700, -42.4500, -73.8000, -42.3900, -73.7400,
     NULL,
     'Conocimiento público; pendiente de validar el decreto y sus límites.')
ON CONFLICT (nombre) DO NOTHING;

COMMIT;
