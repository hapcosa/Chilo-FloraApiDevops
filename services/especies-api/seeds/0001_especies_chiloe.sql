-- =============================================================================
-- 0001_especies_chiloe.sql — contenido inicial de especies del archipiélago
-- =============================================================================
-- Poblado divulgativo de arranque: familias, géneros y especies reales de
-- Chiloé cubriendo los cinco reinos. Sirve tanto para desarrollo local como
-- para el primer contenido de producción.
--
-- Idempotente: se puede reaplicar sin duplicar (ON CONFLICT DO NOTHING sobre
-- las claves únicas naturales). No borra ni modifica lo ya cargado, así que es
-- seguro correrlo sobre una BD con contenido editado a mano.
--
-- Los atributos_especificos cumplen el JSON Schema de cada reino en
-- services/especies-api/config/schemas/. Esos schemas usan
-- additionalProperties:false, de modo que cualquier campo extra hace fallar la
-- validación en la API.
-- =============================================================================

BEGIN;

-- -----------------------------------------------------------------------------
-- Familias
-- -----------------------------------------------------------------------------
INSERT INTO familias (nombre, reino, descripcion) VALUES
    ('Canidae',          'animalia', 'Cánidos: zorros, lobos y perros.'),
    ('Cervidae',         'animalia', 'Cérvidos: ciervos y venados.'),
    ('Rhinocryptidae',   'animalia', 'Tapaculos, aves paseriformes del sotobosque austral.'),
    ('Microbiotheriidae','animalia', 'Marsupiales microbioterios, linaje relicto del bosque templado.'),
    ('Cupressaceae',     'plantae',  'Cipreses y alerces, coníferas de hoja escamosa.'),
    ('Winteraceae',      'plantae',  'Angiospermas basales de madera sin vasos.'),
    ('Gunneraceae',      'plantae',  'Hierbas gigantes de hojas peltadas.'),
    ('Myrtaceae',        'plantae',  'Mirtáceas: arrayanes, tepúes y luma.'),
    ('Cyttariaceae',     'fungi',    'Ascomicetos parásitos obligados de Nothofagus.'),
    ('Gomphaceae',       'fungi',    'Basidiomicetos coraloides.'),
    ('Laminariaceae',    'protista', 'Algas pardas de gran porte formadoras de bosques submarinos.'),
    ('Gonyaulacaceae',   'protista', 'Dinoflagelados marinos, varios formadores de florecimientos nocivos.'),
    ('Nostocaceae',      'monera',   'Cianobacterias filamentosas fijadoras de nitrógeno.')
ON CONFLICT (reino, nombre) DO NOTHING;

-- -----------------------------------------------------------------------------
-- Géneros
-- -----------------------------------------------------------------------------
INSERT INTO generos (nombre, familia_id, descripcion)
SELECT v.nombre, f.id, v.descripcion
FROM (VALUES
    ('Lycalopex',   'animalia', 'Canidae',           'Zorros sudamericanos.'),
    ('Pudu',        'animalia', 'Cervidae',          'Los ciervos más pequeños del mundo.'),
    ('Scelorchilus','animalia', 'Rhinocryptidae',    'Tapaculos de canto sonoro y vuelo torpe.'),
    ('Dromiciops',  'animalia', 'Microbiotheriidae', 'Único género vivo de su orden.'),
    ('Fitzroya',    'plantae',  'Cupressaceae',      'Género monotípico del alerce.'),
    ('Drimys',      'plantae',  'Winteraceae',       'Árboles de corteza aromática.'),
    ('Gunnera',     'plantae',  'Gunneraceae',       'Hierbas de hojas muy grandes.'),
    ('Luma',        'plantae',  'Myrtaceae',         'Mirtáceas de corteza lisa y anaranjada.'),
    ('Cyttaria',    'fungi',    'Cyttariaceae',      'Hongos esféricos sobre ramas de coigüe y roble.'),
    ('Ramaria',     'fungi',    'Gomphaceae',        'Hongos ramificados en forma de coral.'),
    ('Macrocystis', 'protista', 'Laminariaceae',     'Huiros formadores de bosques de kelp.'),
    ('Alexandrium', 'protista', 'Gonyaulacaceae',    'Dinoflagelados productores de toxinas paralizantes.'),
    ('Nostoc',      'monera',   'Nostocaceae',       'Colonias gelatinosas de cianobacterias.')
) AS v(nombre, reino, familia, descripcion)
JOIN familias f ON f.nombre = v.familia AND f.reino = v.reino::reino_enum
ON CONFLICT (familia_id, nombre) DO NOTHING;

-- -----------------------------------------------------------------------------
-- Especies
-- -----------------------------------------------------------------------------
INSERT INTO especies (
    nombre_cientifico, nombre_comun, autor_cientifico, descripcion, habitat,
    distribucion_chiloe, endemica, estado_conservacion, reino, genero_id,
    atributos_especificos, fuentes
)
SELECT
    v.nombre_cientifico, v.nombre_comun, v.autor_cientifico, v.descripcion,
    v.habitat, v.distribucion_chiloe, v.endemica, v.estado_conservacion,
    v.reino::reino_enum, g.id, v.atributos::jsonb, v.fuentes::jsonb
FROM (VALUES
    (
        'Lycalopex fulvipes', 'Zorro de Darwin', '(Martin, 1837)',
        'El cánido más amenazado de Chile. Darwin lo describió en 1834 durante su paso por San Pedro, en el sur de Chiloé. Es más pequeño y oscuro que los otros zorros chilenos, con patas cortas y hocico corto.',
        'Bosque templado lluvioso denso, con preferencia por interior de bosque y bordes.',
        'Presente en gran parte de la Isla Grande; la población insular es la más numerosa de la especie.',
        true, 'En Peligro', 'animalia', 'Lycalopex',
        '{"clase":"Mammalia","alimentacion":"omnivoro","dieta_detalle":"Micromamíferos, aves, insectos, anfibios y una proporción importante de frutos del sotobosque.","comportamiento":{"actividad":"catemeral","social":"solitario","migratorio":false},"tamano_promedio_cm":55,"peso_promedio_g":2900,"reproduccion":"viviparo","epoca_reproductiva":"Camadas en primavera austral, entre octubre y diciembre."}',
        '["IUCN Red List — Lycalopex fulvipes","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Pudu puda', 'Pudú', '(Molina, 1782)',
        'El ciervo más pequeño de Sudamérica: un adulto no supera la altura de la rodilla de una persona. De cuerpo compacto y astas cortas y rectas en los machos.',
        'Sotobosque denso con quila y renovales, donde encuentra refugio y alimento.',
        'Distribuido por toda la Isla Grande, más frecuente donde el bosque conserva sotobosque cerrado.',
        false, 'Casi Amenazado', 'animalia', 'Pudu',
        '{"clase":"Mammalia","alimentacion":"herbivoro","dieta_detalle":"Hojas, brotes tiernos, cortezas y frutos caídos del sotobosque.","comportamiento":{"actividad":"crepuscular","social":"solitario","migratorio":false},"tamano_promedio_cm":80,"peso_promedio_g":9000,"reproduccion":"viviparo","epoca_reproductiva":"Partos entre noviembre y enero, generalmente una cría."}',
        '["IUCN Red List — Pudu puda","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Scelorchilus rubecula', 'Chucao', '(Kittlitz, 1830)',
        'Ave emblemática del bosque austral, de pecho anaranjado intenso. Se escucha mucho más de lo que se ve: su canto fuerte y repentino es parte del paisaje sonoro chilote. La tradición local le atribuye anuncios de buena o mala fortuna según de qué lado cante.',
        'Suelo y sotobosque húmedo del bosque templado, entre helechos y troncos caídos.',
        'Común en bosques de toda la isla, incluidos fragmentos y bordes de camino.',
        false, 'Preocupación Menor', 'animalia', 'Scelorchilus',
        '{"clase":"Aves","alimentacion":"insectivoro","dieta_detalle":"Invertebrados del mantillo: insectos, arañas, lombrices y ocasionalmente semillas.","comportamiento":{"actividad":"diurno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":19,"peso_promedio_g":40,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en primavera, en cavidades y barrancos."}',
        '["IUCN Red List — Scelorchilus rubecula","Aves de Chile — Guía de campo"]'
    ),
    (
        'Dromiciops gliroides', 'Monito del monte', 'Thomas, 1894',
        'Marsupial nocturno, único sobreviviente del orden Microbiotheria, más emparentado con los marsupiales australianos que con los americanos. Entra en torpor durante el invierno acumulando grasa en la cola.',
        'Bosque templado lluvioso con abundante quila y epífitas, donde construye nidos esféricos de hojas.',
        'Presente en sectores boscosos bien conservados de la Isla Grande.',
        false, 'Casi Amenazado', 'animalia', 'Dromiciops',
        '{"clase":"Mammalia","alimentacion":"omnivoro","dieta_detalle":"Insectos y frutos; es el principal dispersor de semillas del quintral y otras plantas del bosque.","comportamiento":{"actividad":"nocturno","social":"solitario","migratorio":false},"tamano_promedio_cm":24,"peso_promedio_g":40,"reproduccion":"viviparo","epoca_reproductiva":"Cría en primavera-verano, con marsupio de cuatro mamas."}',
        '["IUCN Red List — Dromiciops gliroides","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Fitzroya cupressoides', 'Alerce', '(Molina) I.M.Johnst.',
        'Una de las especies arbóreas más longevas del planeta: se han datado ejemplares de más de 3.600 años. Su madera fue tan explotada que llegó a usarse como moneda en Chiloé, en tejuelas conocidas como "real de alerce". Monumento Natural desde 1976.',
        'Suelos húmedos y turbosos, desde el nivel del mar hasta la cordillera.',
        'Poblaciones en la cordillera de Piuchén y sectores de la costa occidental.',
        false, 'En Peligro', 'plantae', 'Fitzroya',
        '{"tipo_planta":"arbol","altura_promedio_m":45,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"polinizacion":"anemofila","tipo_raiz":"axonomorfa","usos_tradicionales":["Tejuelas para revestimiento de casas e iglesias","Construcción de embarcaciones"],"fruto":{"descripcion":"Conos pequeños y globosos de unos 6 mm.","comestible":"no_comestible"}}',
        '["IUCN Red List — Fitzroya cupressoides","CONAF — Monumento Natural Alerce"]'
    ),
    (
        'Drimys winteri', 'Canelo', 'J.R.Forst. & G.Forst.',
        'Árbol sagrado del pueblo mapuche-huilliche, presente en ceremonias y rogativas. Su corteza, rica en vitamina C, fue usada por navegantes contra el escorbuto. De hoja perenne, aromática y de flores blancas.',
        'Suelos húmedos, orillas de cursos de agua, turberas y bordes de bosque.',
        'Muy extendido en toda la isla, incluso en terrenos abiertos y suelos anegados.',
        false, 'Preocupación Menor', 'plantae', 'Drimys',
        '{"tipo_planta":"arbol","altura_promedio_m":20,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[10,11,12,1],"polinizacion":"entomofila","tipo_raiz":"axonomorfa","usos_tradicionales":["Planta ceremonial mapuche-huilliche","Corteza medicinal antiescorbútica"],"fruto":{"descripcion":"Baya pequeña de color negro azulado al madurar.","comestible":"no_comestible"}}',
        '["Flora de Chile — Drimys winteri","Etnobotánica del bosque templado austral"]'
    ),
    (
        'Gunnera tinctoria', 'Nalca', '(Molina) Mirb.',
        'Hierba gigante de hojas que superan el metro de diámetro, sostenidas por pecíolos comestibles. La nalca se come cruda con sal y es ingrediente tradicional de la cocina chilota. Establece simbiosis con cianobacterias del género Nostoc, que le fijan nitrógeno.',
        'Quebradas húmedas, orillas de esteros, taludes y bordes de camino con suelo removido.',
        'Abundante en toda la isla, especialmente visible en cunetas y quebradas.',
        false, NULL, 'plantae', 'Gunnera',
        '{"tipo_planta":"hierba","altura_promedio_m":2,"tipo_hoja":{"ciclo":"caduca","morfologia":"simple"},"floracion_meses":[11,12,1],"polinizacion":"anemofila","tipo_raiz":"rizoma","usos_tradicionales":["Pecíolo comestible crudo o en ensaladas","Hojas para cubrir el curanto en hoyo"],"fruto":{"descripcion":"Infrutescencia cónica con numerosos frutos pequeños anaranjados.","comestible":"comestible"}}',
        '["Flora de Chile — Gunnera tinctoria","Cocina tradicional chilota"]'
    ),
    (
        'Luma apiculata', 'Arrayán', '(DC.) Burret',
        'Árbol de corteza lisa y anaranjada, fría al tacto, que se desprende en placas dejando manchas blanquecinas. Forma bosquetes densos junto a cursos de agua. Sus frutos negros y dulces son alimento de aves y también se consumen frescos.',
        'Riberas de ríos y lagos, suelos húmedos y bien drenados.',
        'Frecuente en bordes de cursos de agua y sectores costeros de la isla.',
        false, 'Preocupación Menor', 'plantae', 'Luma',
        '{"tipo_planta":"arbol","altura_promedio_m":15,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[1,2,3],"polinizacion":"entomofila","tipo_raiz":"axonomorfa","usos_tradicionales":["Frutos comestibles frescos o en mermelada","Leña de alto poder calórico"],"fruto":{"descripcion":"Baya globosa negra de unos 10 mm, dulce al madurar.","comestible":"comestible"}}',
        '["Flora de Chile — Luma apiculata"]'
    ),
    (
        'Cyttaria espinosae', 'Digüeñe', 'Lloyd',
        'Hongo parásito obligado del coigüe, que forma agallas leñosas en las ramas. En primavera emergen cuerpos fructíferos esféricos, anaranjados y de aspecto esponjoso, muy apreciados en la cocina del sur de Chile. Se cosecha silvestre y se vende en ferias durante pocas semanas al año.',
        'Ramas de coigüe (Nothofagus dombeyi) y otras especies del género.',
        'Aparece en bosques de coigüe de la isla durante la primavera.',
        false, NULL, 'fungi', 'Cyttaria',
        '{"tipo":"ascomiceto","comestibilidad":"comestible","simbiosis":"parasito","sustrato":["Ramas vivas de Nothofagus dombeyi"],"tipo_himenio":"gleba","temporada":["primavera"]}',
        '["Hongos de Chile — Cyttaria espinosae","Fundación Fungi — Guía de hongos comestibles"]'
    ),
    (
        'Ramaria flava', 'Changle', '(Schaeff.) Quél.',
        'Hongo de aspecto coraloide, con ramificaciones amarillentas que emergen del mantillo. Es uno de los hongos silvestres más consumidos en el sur de Chile, tradicionalmente salteado o en guisos. Conviene cocerlo bien: crudo o mal cocinado puede provocar molestias digestivas.',
        'Suelo del bosque templado, entre hojarasca y raíces.',
        'En bosques húmedos de la isla, principalmente en otoño.',
        false, NULL, 'fungi', 'Ramaria',
        '{"tipo":"basidiomiceto","comestibilidad":"comestible","simbiosis":"micorrizico","sustrato":["Mantillo y suelo de bosque templado"],"tipo_himenio":"liso","temporada":["otoño"],"advertencia":"Debe consumirse bien cocido. Varias especies de Ramaria son muy similares entre sí y algunas provocan trastornos gastrointestinales, por lo que la identificación debe confirmarla una persona con experiencia."}',
        '["Hongos de Chile — Ramaria flava","Fundación Fungi — Guía de hongos comestibles"]'
    ),
    (
        'Macrocystis pyrifera', 'Huiro', '(L.) C.Agardh',
        'Alga parda gigante que forma bosques submarinos de hasta 45 metros, sostenida por flotadores llenos de gas. Estos bosques dan refugio y alimento a peces, crustáceos y moluscos, y amortiguan el oleaje en la costa. Se cosecha para extraer alginatos.',
        'Fondos rocosos submareales de aguas frías, bien oxigenadas y con movimiento.',
        'Costas expuestas del archipiélago, especialmente en el sector occidental.',
        false, NULL, 'protista', 'Macrocystis',
        '{"grupo":"algas_pardas","ambiente":"marino","morfologia":"talo","tamano_promedio_mm":30000,"importancia_ecologica":"Forma bosques de kelp que sirven de hábitat, refugio y zona de crianza para gran parte de la fauna costera; además captura carbono y reduce la energía del oleaje sobre la costa."}',
        '["Algas marinas de Chile — Macrocystis pyrifera","IFOP — Praderas de huiro"]'
    ),
    (
        'Alexandrium catenella', 'Dinoflagelado de la marea roja', '(Whedon & Kof.) Balech',
        'Dinoflagelado microscópico responsable de los episodios de marea roja en el sur de Chile. Produce toxinas paralizantes que se acumulan en mariscos filtradores sin alterar su aspecto ni su sabor, y que no se destruyen al cocinarlos. Sus florecimientos obligan a cerrar áreas de extracción y son monitoreados de forma permanente.',
        'Aguas marinas y estuarinas; forma quistes de resistencia en el sedimento.',
        'Florecimientos recurrentes en los canales y el mar interior del archipiélago.',
        false, NULL, 'protista', 'Alexandrium',
        '{"grupo":"dinoflagelados","ambiente":"marino","morfologia":"unicelular","tamano_promedio_mm":0.04,"importancia_ecologica":"Genera floraciones algales nocivas con toxina paralizante. Los mariscos afectados no presentan cambios visibles y la toxina resiste la cocción, por lo que solo el monitoreo sanitario oficial permite determinar si el consumo es seguro."}',
        '["IFOP — Programa de Marea Roja","Servicio Nacional de Pesca — Vedas por floraciones algales nocivas"]'
    ),
    (
        'Nostoc commune', 'Llullucha', 'Vaucher ex Bornet & Flahault',
        'Cianobacteria que forma colonias gelatinosas verde oliva sobre suelos húmedos, visibles sobre todo después de la lluvia. Fija nitrógeno atmosférico, enriqueciendo el suelo, y establece simbiosis con la nalca alojándose en la base de sus pecíolos.',
        'Suelos húmedos, praderas anegadas, roca desnuda y bordes de camino.',
        'Amplia presencia en toda la isla, favorecida por el clima lluvioso.',
        false, NULL, 'monera', 'Nostoc',
        '{"dominio":"bacteria","forma":"filamentoso","gram":"negativo","metabolismo":{"fuente_energia":"autotrofo","oxigeno":"aerobio_estricto"},"relevancia_chiloe":"Fija nitrógeno atmosférico y lo aporta al suelo. Vive en simbiosis con la nalca (Gunnera tinctoria), alojándose en la base de sus pecíolos, lo que explica en parte el vigor de esta planta en suelos pobres."}',
        '["Cianobacterias de Chile — Nostoc commune","Simbiosis Gunnera-Nostoc: revisión"]'
    )
) AS v(
    nombre_cientifico, nombre_comun, autor_cientifico, descripcion, habitat,
    distribucion_chiloe, endemica, estado_conservacion, reino, genero,
    atributos, fuentes
)
JOIN generos g ON g.nombre = v.genero
JOIN familias f ON f.id = g.familia_id AND f.reino = v.reino::reino_enum
ON CONFLICT (nombre_cientifico) DO NOTHING;

COMMIT;
