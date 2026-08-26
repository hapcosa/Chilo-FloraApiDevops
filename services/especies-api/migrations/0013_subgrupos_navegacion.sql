-- =============================================================================
-- 0013_subgrupos_navegacion.sql
-- =============================================================================
-- Subgrupos dentro de cada reino, para que la app pueda filtrar más fino que
-- "animalia" (docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md, Fase 9.4, PR 15).
--
-- Se reusa `categorias_moderacion` como eje único de subgrupo en vez de crear
-- una tabla `clases` paralela. La consecuencia, decidida a ojos abiertos: el
-- eje de navegación y el de permisos de curaduría son el mismo registro, así
-- que crear la pestaña "Aves" crea también la unidad curable "Aves". Se acepta
-- porque hoy coinciden —quien cura aves es quien sabe de aves— y porque dos
-- particiones parecidas conviviendo terminan contradiciéndose. La regla que
-- queda: la navegación no inventa agrupaciones que no sean curables.
--
-- Corrige un supuesto del 0004, que decía que las subcategorías reales las
-- crean los admins por la API. No sirve para éstas: tienen que existir
-- idénticas en todos los entornos, que es justo lo que garantiza una migración
-- y no un script que alguien recuerda correr. Las que sí crearán los admins
-- son las que salgan de fichas futuras.
--
-- De dónde salen los grupos (no de la intuición):
--   * animalia — los seis del Reglamento de Clasificación de Especies del MMA:
--     aves, mamíferos, reptiles, anfibios, peces, invertebrados.
--   * plantae — por división; en Chiloé coinciden con lo que se distingue
--     caminando (alerce y ciprés vs. helechos vs. pomponal).
--   * fungi — el MMA nombra "hongos y líquenes" separados en sus procesos.
--   * protista — macroalgas partidas en pardas/rojas y las microalgas aparte,
--     según la clasificación del Museo de Historia Natural de Concepción. Los
--     dinoflagelados van con las microalgas porque es el grupo de la marea
--     roja, que acá no es un tecnicismo.
--   * monera — se queda con su categoría "general". Sus seis fichas son
--     cianobacterias y patógenos de salmónidos: un selector de dos opciones
--     sería ruido antes que navegación. Se parte cuando haya fichas que lo
--     justifiquen.
--
-- "Peces" se crea vacío a propósito: es parte de la partición del MMA y las
-- fichas van a llegar. La app esconde los grupos sin fichas, así que no deja
-- un callejón sin salida en la pantalla.
--
-- La clasificación es por familia, que es un dato que cada ficha ya tiene: no
-- hay criterio a ojo. Lo que no matchee ninguna familia conocida se queda en la
-- categoría "general" de su reino, que es donde ya está.
--
-- El mapeo familia → subgrupo vive en su propia tabla y no embebido en un
-- UPDATE. Las migraciones corren *antes* que los seeds, así que un entorno
-- nuevo inserta las fichas después de este backfill y quedarían sin subgrupo;
-- con la tabla, el seed reaplica la misma clasificación sin copiar la lista, y
-- las fichas que se agreguen después tienen de dónde deducir su grupo.
-- =============================================================================

BEGIN;

INSERT INTO categorias_moderacion (slug, nombre, reino, descripcion) VALUES
    ('animalia-aves',           'Aves',           'animalia',
     'Aves terrestres y marinas del archipiélago.'),
    ('animalia-mamiferos',      'Mamíferos',      'animalia',
     'Mamíferos terrestres y marinos, incluidos cetáceos y pinnípedos.'),
    ('animalia-reptiles',       'Reptiles',       'animalia',
     'Lagartijas, culebras y demás reptiles.'),
    ('animalia-anfibios',       'Anfibios',       'animalia',
     'Ranas y sapos, el grupo con mayor endemismo del país.'),
    ('animalia-peces',          'Peces',          'animalia',
     'Peces marinos y de aguas continentales.'),
    ('animalia-invertebrados',  'Invertebrados',  'animalia',
     'Moluscos, crustáceos, insectos, equinodermos y otros invertebrados.'),

    ('plantae-coniferas',       'Coníferas',      'plantae',
     'Alerce, ciprés de las Guaitecas, mañío y demás gimnospermas.'),
    ('plantae-flores',          'Plantas con flores', 'plantae',
     'Angiospermas: árboles, arbustos y hierbas con flor.'),
    ('plantae-helechos',        'Helechos',       'plantae',
     'Helechos y plantas afines.'),
    ('plantae-musgos',          'Musgos',         'plantae',
     'Musgos y hepáticas, incluido el pompón de los pomponales.'),

    ('fungi-hongos',            'Hongos',         'fungi',
     'Hongos con cuerpo fructífero visible.'),
    ('fungi-liquenes',          'Líquenes',       'fungi',
     'Líquenes: la asociación entre un hongo y un alga o cianobacteria.'),

    ('protista-algas-pardas',   'Algas pardas',   'protista',
     'Huiros y cochayuyo: las macroalgas que forman los bosques submarinos.'),
    ('protista-algas-rojas',    'Algas rojas',    'protista',
     'Macroalgas rojas, entre ellas el pelillo.'),
    ('protista-microalgas',     'Microalgas y dinoflagelados', 'protista',
     'Microalgas del plancton, incluidas las que provocan la marea roja.')
ON CONFLICT (slug) DO NOTHING;

-- -----------------------------------------------------------------------------
-- Mapeo familia → subgrupo
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS familia_subgrupo (
    familia      VARCHAR(100) PRIMARY KEY,
    categoria_id INTEGER NOT NULL
        REFERENCES categorias_moderacion(id) ON DELETE CASCADE
);

COMMENT ON TABLE familia_subgrupo IS
    'A qué subgrupo de navegación pertenece cada familia taxonómica. Referencia, no contenido: la usan el backfill de 0013 y los seeds para clasificar sin repetir la lista.';

INSERT INTO familia_subgrupo (familia, categoria_id)
SELECT m.familia, c.id
FROM (VALUES
    -- animalia · aves
    ('Anatidae', 'animalia-aves'),
    ('Cathartidae', 'animalia-aves'),
    ('Falconidae', 'animalia-aves'),
    ('Furnariidae', 'animalia-aves'),
    ('Haematopodidae', 'animalia-aves'),
    ('Icteridae', 'animalia-aves'),
    ('Laridae', 'animalia-aves'),
    ('Pelecanoididae', 'animalia-aves'),
    ('Phalacrocoracidae', 'animalia-aves'),
    ('Picidae', 'animalia-aves'),
    ('Psittacidae', 'animalia-aves'),
    ('Rhinocryptidae', 'animalia-aves'),
    ('Spheniscidae', 'animalia-aves'),
    ('Strigidae', 'animalia-aves'),
    ('Thraupidae', 'animalia-aves'),
    ('Threskiornithidae', 'animalia-aves'),
    ('Trochilidae', 'animalia-aves'),
    ('Turdidae', 'animalia-aves'),
    ('Tyrannidae', 'animalia-aves'),
    ('Tytonidae', 'animalia-aves'),
    -- animalia · mamíferos
    ('Balaenidae', 'animalia-mamiferos'),
    ('Balaenopteridae', 'animalia-mamiferos'),
    ('Canidae', 'animalia-mamiferos'),
    ('Cervidae', 'animalia-mamiferos'),
    ('Cricetidae', 'animalia-mamiferos'),
    ('Delphinidae', 'animalia-mamiferos'),
    ('Echimyidae', 'animalia-mamiferos'),
    ('Felidae', 'animalia-mamiferos'),
    ('Mephitidae', 'animalia-mamiferos'),
    ('Microbiotheriidae', 'animalia-mamiferos'),
    ('Mustelidae', 'animalia-mamiferos'),
    ('Otariidae', 'animalia-mamiferos'),
    ('Vespertilionidae', 'animalia-mamiferos'),
    -- animalia · reptiles
    ('Colubridae', 'animalia-reptiles'),
    ('Liolaemidae', 'animalia-reptiles'),
    -- animalia · anfibios
    ('Alsodidae', 'animalia-anfibios'),
    ('Batrachylidae', 'animalia-anfibios'),
    ('Rhinodermatidae', 'animalia-anfibios'),
    -- animalia · invertebrados
    ('Carabidae', 'animalia-invertebrados'),
    ('Lucanidae', 'animalia-invertebrados'),
    ('Muricidae', 'animalia-invertebrados'),
    ('Mytilidae', 'animalia-invertebrados'),
    ('Parechinidae', 'animalia-invertebrados'),
    ('Veneridae', 'animalia-invertebrados'),
    -- plantae
    ('Cupressaceae', 'plantae-coniferas'),
    ('Podocarpaceae', 'plantae-coniferas'),
    ('Blechnaceae', 'plantae-helechos'),
    ('Dicksoniaceae', 'plantae-helechos'),
    ('Sphagnaceae', 'plantae-musgos'),
    ('Aextoxicaceae', 'plantae-flores'),
    ('Alstroemeriaceae', 'plantae-flores'),
    ('Atherospermataceae', 'plantae-flores'),
    ('Berberidaceae', 'plantae-flores'),
    ('Columelliaceae', 'plantae-flores'),
    ('Cunoniaceae', 'plantae-flores'),
    ('Ericaceae', 'plantae-flores'),
    ('Gunneraceae', 'plantae-flores'),
    ('Myrtaceae', 'plantae-flores'),
    ('Nothofagaceae', 'plantae-flores'),
    ('Onagraceae', 'plantae-flores'),
    ('Philesiaceae', 'plantae-flores'),
    ('Poaceae', 'plantae-flores'),
    ('Proteaceae', 'plantae-flores'),
    ('Winteraceae', 'plantae-flores'),
    -- fungi
    ('Amanitaceae', 'fungi-hongos'),
    ('Cyttariaceae', 'fungi-hongos'),
    ('Fistulinaceae', 'fungi-hongos'),
    ('Gomphaceae', 'fungi-hongos'),
    ('Meripilaceae', 'fungi-hongos'),
    ('Lobariaceae', 'fungi-liquenes'),
    ('Parmeliaceae', 'fungi-liquenes'),
    -- protista
    ('Durvillaeaceae', 'protista-algas-pardas'),
    ('Laminariaceae', 'protista-algas-pardas'),
    ('Lessoniaceae', 'protista-algas-pardas'),
    ('Gracilariaceae', 'protista-algas-rojas'),
    ('Gonyaulacaceae', 'protista-microalgas')
) AS m(familia, slug)
JOIN categorias_moderacion c ON c.slug = m.slug
ON CONFLICT (familia) DO NOTHING;

-- -----------------------------------------------------------------------------
-- Backfill de las fichas que ya existen
-- -----------------------------------------------------------------------------
UPDATE especies e
SET categoria_id = fs.categoria_id
FROM generos g
JOIN familias f          ON f.id = g.familia_id
JOIN familia_subgrupo fs ON fs.familia = f.nombre
JOIN categorias_moderacion c ON c.id = fs.categoria_id
WHERE e.genero_id = g.id
  -- La categoría del reino tiene que coincidir: una familia mal clasificada en
  -- la BD no debe arrastrar la ficha a un subgrupo de otro reino.
  AND c.reino = e.reino;

COMMIT;
