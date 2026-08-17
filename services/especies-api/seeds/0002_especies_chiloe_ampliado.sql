-- =============================================================================
-- 0002_especies_chiloe_ampliado.sql — ampliación del catálogo a ~100 especies
-- =============================================================================
-- Continúa el poblado de 0001_especies_chiloe.sql con el resto de la biota
-- característica del archipiélago, cubriendo los cinco reinos. Las 13 especies
-- del seed anterior no se repiten aquí.
--
-- Idempotente igual que el 0001: ON CONFLICT DO NOTHING sobre las claves
-- únicas naturales (familias(reino,nombre), generos(familia_id,nombre),
-- especies.nombre_cientifico). Reaplicarlo deja la BD igual.
--
-- Los atributos_especificos cumplen el JSON Schema de cada reino en
-- services/especies-api/config/schemas/, que usan additionalProperties:false.
--
-- Las descripciones son divulgativas pero con detalle morfológico y ecológico:
-- el catálogo se consulta como material de referencia, no como pie de foto.
-- =============================================================================

BEGIN;

-- =============================================================================
-- ANIMALIA
-- =============================================================================

INSERT INTO familias (nombre, reino, descripcion) VALUES
    ('Cathartidae',        'animalia', 'Buitres del Nuevo Mundo, planeadores de alas anchas y olfato desarrollado.'),
    ('Threskiornithidae',  'animalia', 'Ibis y espátulas, aves zancudas de pico especializado.'),
    ('Anatidae',           'animalia', 'Cisnes, gansos y patos.'),
    ('Spheniscidae',       'animalia', 'Pingüinos, aves marinas no voladoras del hemisferio sur.'),
    ('Pelecanoididae',     'animalia', 'Yuncos o petreles buceadores, de vuelo zumbante y buceo con alas.'),
    ('Phalacrocoracidae',  'animalia', 'Cormoranes, buceadores de plumaje poco impermeable.'),
    ('Laridae',            'animalia', 'Gaviotas y gaviotines.'),
    ('Haematopodidae',     'animalia', 'Ostreros, aves costeras de pico rojo comprimido.'),
    ('Trochilidae',        'animalia', 'Picaflores, aves nectarívoras de vuelo estacionario.'),
    ('Psittacidae',        'animalia', 'Loros y cotorras del Nuevo Mundo.'),
    ('Picidae',            'animalia', 'Carpinteros, aves trepadoras excavadoras de madera.'),
    ('Strigidae',          'animalia', 'Búhos y lechuzas de penacho auricular.'),
    ('Tytonidae',          'animalia', 'Lechuzas de disco facial acorazonado.'),
    ('Icteridae',          'animalia', 'Tordos, loicas y afines.'),
    ('Turdidae',           'animalia', 'Zorzales, paseriformes omnívoros de suelo.'),
    ('Tyrannidae',         'animalia', 'Atrapamoscas del Nuevo Mundo.'),
    ('Furnariidae',        'animalia', 'Horneros y afines, grandes constructores de nidos.'),
    ('Thraupidae',         'animalia', 'Tángaras y semilleros neotropicales.'),
    ('Falconidae',         'animalia', 'Halcones, caranchos y tiuques.'),
    ('Mustelidae',         'animalia', 'Nutrias, hurones y visones.'),
    ('Felidae',            'animalia', 'Félidos.'),
    ('Mephitidae',         'animalia', 'Zorrinos y chingues, de defensa química.'),
    ('Echimyidae',         'animalia', 'Roedores espinosos neotropicales; incluye al coipo.'),
    ('Cricetidae',         'animalia', 'Ratones y ratas del Nuevo Mundo.'),
    ('Otariidae',          'animalia', 'Lobos marinos y osos marinos, pinnípedos con orejas externas.'),
    ('Balaenidae',         'animalia', 'Ballenas francas, sin aleta dorsal y de barbas largas.'),
    ('Balaenopteridae',    'animalia', 'Rorcuales, ballenas de surcos ventrales.'),
    ('Delphinidae',        'animalia', 'Delfines oceánicos.'),
    ('Vespertilionidae',   'animalia', 'Murciélagos insectívoros de cola incluida en el uropatagio.'),
    ('Rhinodermatidae',    'animalia', 'Ranitas de Darwin, únicas por su neomelia bucal.'),
    ('Batrachylidae',      'animalia', 'Anuros del bosque templado austral.'),
    ('Alsodidae',          'animalia', 'Sapitos de hojarasca del bosque valdiviano.'),
    ('Liolaemidae',        'animalia', 'Lagartijas sudamericanas del cono sur.'),
    ('Colubridae',         'animalia', 'Culebras, el mayor grupo de serpientes.'),
    ('Muricidae',          'animalia', 'Caracoles marinos carnívoros de concha gruesa.'),
    ('Mytilidae',          'animalia', 'Mitílidos: choritos, cholgas y choros.'),
    ('Veneridae',          'animalia', 'Almejas de concha equivalva y sifones cortos.'),
    ('Parechinidae',       'animalia', 'Erizos de mar regulares.'),
    ('Carabidae',          'animalia', 'Escarabajos corredores, depredadores del suelo.'),
    ('Lucanidae',          'animalia', 'Escarabajos ciervo, de mandíbulas hipertrofiadas en los machos.')
ON CONFLICT (reino, nombre) DO NOTHING;

INSERT INTO generos (nombre, familia_id, descripcion)
SELECT v.nombre, f.id, v.descripcion
FROM (VALUES
    ('Vultur',           'animalia', 'Cathartidae',       'Género monotípico del cóndor andino.'),
    ('Theristicus',      'animalia', 'Threskiornithidae', 'Bandurrias de pico curvo y cuello ocráceo.'),
    ('Cygnus',           'animalia', 'Anatidae',          'Cisnes.'),
    ('Coscoroba',        'animalia', 'Anatidae',          'Género monotípico intermedio entre cisnes y gansos.'),
    ('Spheniscus',       'animalia', 'Spheniscidae',      'Pingüinos de banda pectoral y clima templado.'),
    ('Pelecanoides',     'animalia', 'Pelecanoididae',    'Petreles buceadores.'),
    ('Phalacrocorax',    'animalia', 'Phalacrocoracidae', 'Cormoranes de agua dulce y salobre.'),
    ('Larus',            'animalia', 'Laridae',           'Gaviotas de gran porte.'),
    ('Haematopus',       'animalia', 'Haematopodidae',    'Ostreros.'),
    ('Pteroptochos',     'animalia', 'Rhinocryptidae',    'Hued-hueds, tapaculos de gran tamaño.'),
    ('Sephanoides',      'animalia', 'Trochilidae',       'Picaflores australes.'),
    ('Enicognathus',     'animalia', 'Psittacidae',       'Cotorras australes de cola larga.'),
    ('Campephilus',      'animalia', 'Picidae',           'Carpinteros grandes de cresta roja.'),
    ('Strix',            'animalia', 'Strigidae',         'Búhos sin penachos, de disco facial marcado.'),
    ('Glaucidium',       'animalia', 'Strigidae',         'Chunchos, búhos diminutos y diurnos.'),
    ('Tyto',             'animalia', 'Tytonidae',         'Lechuzas de campanario.'),
    ('Curaeus',          'animalia', 'Icteridae',         'Tordos australes de plumaje negro.'),
    ('Turdus',           'animalia', 'Turdidae',          'Zorzales.'),
    ('Elaenia',          'animalia', 'Tyrannidae',        'Fío-fíos, atrapamoscas migratorios.'),
    ('Aphrastura',       'animalia', 'Furnariidae',       'Rayaditos, furnáridos de cola espinosa.'),
    ('Sylviorthorhynchus','animalia','Furnariidae',       'Colilargas, de cola desproporcionada.'),
    ('Phrygilus',        'animalia', 'Thraupidae',        'Cometocinos y afines.'),
    ('Milvago',          'animalia', 'Falconidae',        'Tiuques, falcónidos oportunistas.'),
    ('Lontra',           'animalia', 'Mustelidae',        'Nutrias americanas.'),
    ('Puma',             'animalia', 'Felidae',           'Género monotípico del puma.'),
    ('Leopardus',        'animalia', 'Felidae',           'Félidos pequeños neotropicales.'),
    ('Conepatus',        'animalia', 'Mephitidae',        'Chingues de hocico de cerdo.'),
    ('Myocastor',        'animalia', 'Echimyidae',        'Género monotípico del coipo.'),
    ('Oligoryzomys',     'animalia', 'Cricetidae',        'Ratones colilargos.'),
    ('Otaria',           'animalia', 'Otariidae',         'Género monotípico del lobo marino común.'),
    ('Eubalaena',        'animalia', 'Balaenidae',        'Ballenas francas.'),
    ('Balaenoptera',     'animalia', 'Balaenopteridae',   'Rorcuales.'),
    ('Cephalorhynchus',  'animalia', 'Delphinidae',       'Delfines pequeños de aleta redondeada.'),
    ('Lagenorhynchus',   'animalia', 'Delphinidae',       'Delfines de hocico corto y patrón bicolor.'),
    ('Myotis',           'animalia', 'Vespertilionidae',  'Murciélagos oreja de ratón.'),
    ('Rhinoderma',       'animalia', 'Rhinodermatidae',   'Ranitas de Darwin.'),
    ('Batrachyla',       'animalia', 'Batrachylidae',     'Sapitos de hojarasca.'),
    ('Eupsophus',        'animalia', 'Alsodidae',         'Sapitos de cuatro ojos.'),
    ('Liolaemus',        'animalia', 'Liolaemidae',       'Lagartijas del cono sur.'),
    ('Tachymenis',       'animalia', 'Colubridae',        'Culebras de cola corta, opistoglifas.'),
    ('Concholepas',      'animalia', 'Muricidae',         'Género monotípico del loco.'),
    ('Mytilus',          'animalia', 'Mytilidae',         'Choritos.'),
    ('Choromytilus',     'animalia', 'Mytilidae',         'Choros zapato.'),
    ('Aulacomya',        'animalia', 'Mytilidae',         'Cholgas.'),
    ('Ameghinomya',      'animalia', 'Veneridae',         'Almejas australes.'),
    ('Loxechinus',       'animalia', 'Parechinidae',      'Erizos rojos del Pacífico sur.'),
    ('Ceroglossus',      'animalia', 'Carabidae',         'Carábidos iridiscentes endémicos del bosque templado.'),
    ('Chiasognathus',    'animalia', 'Lucanidae',         'Ciervos volantes australes.')
) AS v(nombre, reino, familia, descripcion)
JOIN familias f ON f.nombre = v.familia AND f.reino = v.reino::reino_enum
ON CONFLICT (familia_id, nombre) DO NOTHING;

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
        'Vultur gryphus', 'Cóndor andino', 'Linnaeus, 1758',
        'Ave voladora de mayor superficie alar del planeta, con una envergadura que supera los tres metros. El plumaje adulto es negro con un collar blanco de plumón y grandes parches alares blancos, más extensos en el macho, que además porta una carúncula carnosa sobre el pico. La cabeza desnuda es una adaptación higiénica a la alimentación carroñera. Depende casi por completo del planeo: aprovecha corrientes térmicas y de ladera para recorrer más de doscientos kilómetros diarios con un batir de alas mínimo. Su biología reproductiva es extremadamente lenta —un huevo cada dos años y madurez sexual hacia los ocho— lo que hace que cualquier mortalidad adicional de adultos, sobre todo por cebos envenenados, tenga efecto poblacional desproporcionado.',
        'Acantilados costeros y farellones con corrientes ascendentes; forrajea sobre praderas, playas y borde de bosque.',
        'Observado sobre la costa occidental y los acantilados del Pacífico, con avistamientos regulares aunque no abundantes en la Isla Grande.',
        false, 'Vulnerable', 'animalia', 'Vultur',
        '{"clase":"Aves","alimentacion":"carronero","dieta_detalle":"Carroña de mamíferos medianos y grandes; en la costa aprovecha varamientos de lobos marinos y cetáceos. Localiza el alimento por vista, siguiendo también a otras carroñeras.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":110,"peso_promedio_g":11000,"reproduccion":"oviparo","epoca_reproductiva":"Pone un único huevo cada dos años, en repisas de acantilado inaccesibles."}',
        '["IUCN Red List — Vultur gryphus","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Theristicus melanopis', 'Bandurria', '(Gmelin, 1789)',
        'Ave zancuda robusta de cuello y pecho ocráceos, dorso gris plateado y vientre negro, con un pico largo, curvo y oscuro que usa como sonda. Las patas rojizas y el antifaz de piel desnuda alrededor del ojo completan un aspecto inconfundible. Se desplaza en bandadas ruidosas cuyo llamado metálico y repetido se oye a gran distancia, sobre todo al amanecer y al atardecer cuando se mueve entre dormideros y áreas de alimentación. Forrajea caminando por praderas húmedas y sondeando el suelo blando en busca de invertebrados. Se ha beneficiado de la expansión de la pradera ganadera, y hoy es una de las aves grandes más visibles del paisaje rural chilote.',
        'Praderas húmedas, turberas, orillas de estuario y campos abiertos con suelo blando.',
        'Frecuente en el sector oriental de la isla y en las praderas asociadas a la ruta 5, en bandadas de decenas de individuos.',
        false, 'Preocupación Menor', 'animalia', 'Theristicus',
        '{"clase":"Aves","alimentacion":"insectivoro","dieta_detalle":"Larvas de coleópteros, lombrices y otros invertebrados del suelo, obtenidos sondeando con el pico; ocasionalmente pequeños vertebrados.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":75,"peso_promedio_g":1900,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en primavera, en acantilados o árboles altos, con dos a tres huevos."}',
        '["IUCN Red List — Theristicus melanopis","Aves de Chile — Guía de campo"]'
    ),
    (
        'Cygnus melancoryphus', 'Cisne de cuello negro', '(Molina, 1782)',
        'El mayor anseriforme de Sudamérica y una de las aves más reconocibles de los humedales australes: cuerpo blanco puro contrastado con cabeza y cuello negros, y una carúncula roja en la base del pico gris azulado. Es marcadamente acuático y torpe en tierra, porque las patas están retrasadas respecto del centro de masa; despega con una carrera larga sobre el agua. Se alimenta filtrando y ramoneando vegetación sumergida, por lo que su presencia depende de humedales con buena penetración de luz. Es sensible a los cambios de calidad del agua: episodios de pérdida masiva de macrófitas han provocado mortalidades y emigraciones masivas documentadas en el sur de Chile.',
        'Humedales someros, estuarios, lagunas costeras y bahías protegidas con vegetación sumergida abundante.',
        'Presente en humedales costeros y estuarios del este de la isla, con concentraciones estacionales notables.',
        false, 'Preocupación Menor', 'animalia', 'Cygnus',
        '{"clase":"Aves","alimentacion":"herbivoro","dieta_detalle":"Macrófitas sumergidas, algas filamentosas e invertebrados asociados que ingiere incidentalmente al ramonear.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":120,"peso_promedio_g":4500,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica entre julio y noviembre sobre plataformas flotantes de vegetación; los polluelos viajan sobre el dorso de los padres."}',
        '["IUCN Red List — Cygnus melancoryphus","Aves de Chile — Guía de campo"]'
    ),
    (
        'Spheniscus magellanicus', 'Pingüino de Magallanes', '(J.R. Forster, 1781)',
        'Pingüino de talla media identificable por las dos bandas negras que cruzan el pecho blanco y por la franja blanca que rodea la cara desde el ojo hasta la garganta. Nidifica en cuevas que excava en suelo blando o bajo matorral, lo que lo hace dependiente de islotes con cubierta vegetal y sin depredadores terrestres introducidos. Fuera de la temporada reproductiva es pelágico y se dispersa miles de kilómetros hacia el norte siguiendo cardúmenes. En el archipiélago comparte colonias con el pingüino de Humboldt, una simpatría poco frecuente que convierte a las islas Puñihuil en un sitio de interés internacional para su estudio.',
        'Islotes costeros con suelo excavable y cobertura de matorral o pasto; se alimenta en aguas costeras y de plataforma.',
        'Coloniza los islotes de Puñihuil, en la costa occidental, junto al pingüino de Humboldt.',
        false, 'Preocupación Menor', 'animalia', 'Spheniscus',
        '{"clase":"Aves","alimentacion":"piscivoro","dieta_detalle":"Peces pelágicos pequeños como sardina y anchoveta, complementados con calamares y crustáceos, capturados en buceos de persecución.","comportamiento":{"actividad":"diurno","social":"colonial","migratorio":true},"tamano_promedio_cm":70,"peso_promedio_g":4000,"reproduccion":"oviparo","epoca_reproductiva":"Regresa a la colonia en septiembre; pone dos huevos en cueva y ambos padres se turnan la incubación."}',
        '["IUCN Red List — Spheniscus magellanicus","Aves de Chile — Guía de campo"]'
    ),
    (
        'Spheniscus humboldti', 'Pingüino de Humboldt', 'Meyen, 1834',
        'Pingüino costero asociado a la corriente fría de Humboldt, algo más robusto que su congénere magallánico y separable por su única banda pectoral negra y por la extensa piel rosada desnuda en la base del pico, que le sirve para disipar calor. Su distribución reproductiva está ligada a sustratos donde pueda excavar o refugiarse: guano endurecido, grietas de roca o cuevas. Es la especie más amenazada del género: depende de un puñado de colonias, sufre la sobrepesca de sus presas, la mortalidad incidental en redes y el efecto de los eventos El Niño, que desplazan el alimento y provocan fracasos reproductivos generalizados.',
        'Islotes y roqueríos costeros con grietas o guano excavable; forrajea en aguas frías costeras.',
        'Presente en los islotes de Puñihuil, que constituyen el límite austral de su área reproductiva.',
        false, 'Vulnerable', 'animalia', 'Spheniscus',
        '{"clase":"Aves","alimentacion":"piscivoro","dieta_detalle":"Peces pelágicos de cardumen, principalmente anchoveta y sardina, además de calamares pequeños.","comportamiento":{"actividad":"diurno","social":"colonial","migratorio":false},"tamano_promedio_cm":68,"peso_promedio_g":4200,"reproduccion":"oviparo","epoca_reproductiva":"Puede reproducirse dos veces al año donde el alimento es estable; pone dos huevos por nidada."}',
        '["IUCN Red List — Spheniscus humboldti","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Pelecanoides garnotii', 'Yunco', '(Lesson, 1828)',
        'Ave marina pequeña y compacta, de dorso negro y vientre blanco, con alas cortas y rígidas que usa indistintamente para volar y para propulsarse bajo el agua: entra al mar sin plegarlas y literalmente vuela sumergida. Su vuelo aéreo es bajo, recto y zumbante, muy distinto del planeo de otras aves marinas. Nidifica en madrigueras que excava en depósitos de guano o suelo blando de islotes, hábito que la hizo extremadamente vulnerable a la extracción industrial de guano en el siglo XIX y XX, causa principal del colapso de sus poblaciones. Hoy sobrevive en pocas colonias y su presencia austral es escasa y localizada.',
        'Islotes oceánicos con suelo blando para excavar; forrajea en aguas costeras frías y productivas.',
        'Registros escasos frente a la costa occidental; no se conocen colonias reproductivas consolidadas en la isla.',
        false, 'En Peligro', 'animalia', 'Pelecanoides',
        '{"clase":"Aves","alimentacion":"filtrador","dieta_detalle":"Zooplancton, especialmente eufáusidos y crustáceos pequeños, capturados mediante buceo propulsado con las alas.","comportamiento":{"actividad":"nocturno","social":"colonial","migratorio":false},"tamano_promedio_cm":22,"peso_promedio_g":200,"reproduccion":"oviparo","epoca_reproductiva":"Pone un solo huevo en madriguera; visita la colonia de noche para evitar depredadores."}',
        '["IUCN Red List — Pelecanoides garnotii","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Phalacrocorax brasilianus', 'Yeco', '(Gmelin, 1789)',
        'Cormorán oscuro y esbelto, de cola larga y borde gular blanco y puntiagudo en los adultos reproductivos. A diferencia de la mayoría de las aves acuáticas, su plumaje no es plenamente impermeable: al mojarse pierde flotabilidad, lo que reduce el gasto energético del buceo pero obliga al comportamiento característico de secado con las alas extendidas al sol sobre postes, rocas o boyas. Es la especie de cormorán más tolerante al agua dulce y salobre, y aprovecha por igual ríos, lagos, estuarios y bahías. Nidifica en colonias arbóreas o rocosas, con frecuencia mixtas con garzas.',
        'Estuarios, bahías protegidas, ríos y lagos; percha en estructuras artificiales, boyas y balsas de cultivo.',
        'Abundante en todo el litoral interior del archipiélago y en el estuario de los ríos principales.',
        false, 'Preocupación Menor', 'animalia', 'Phalacrocorax',
        '{"clase":"Aves","alimentacion":"piscivoro","dieta_detalle":"Peces bentónicos y de media agua capturados en buceo con propulsión de patas; complementa con crustáceos.","comportamiento":{"actividad":"diurno","social":"colonial","migratorio":false},"tamano_promedio_cm":68,"peso_promedio_g":1400,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en colonias sobre árboles o roqueríos, con tres a cinco huevos."}',
        '["IUCN Red List — Nannopterum brasilianum","Aves de Chile — Guía de campo"]'
    ),
    (
        'Larus dominicanus', 'Gaviota dominicana', 'Lichtenstein, 1823',
        'La gaviota grande más común del hemisferio sur, de manto y alas negras contrastadas con cuerpo blanco, pico amarillo con mancha roja subterminal y patas verdosas. Los juveniles son pardos y jaspeados y tardan tres o cuatro años en adquirir el plumaje adulto, lo que suele confundir a quien recién empieza a observar aves. Es un oportunista extremo: sigue embarcaciones, aprovecha descartes de pesca y desechos de plantas procesadoras, y depreda huevos y polluelos de otras aves marinas. Esa plasticidad le ha permitido aumentar donde la actividad humana concentra alimento, con efectos negativos documentados sobre colonias de especies más especializadas.',
        'Litoral completo: playas, roqueríos, puertos, vertederos y áreas de descarte pesquero.',
        'Abundante y ubicua en toda la costa del archipiélago, especialmente en caletas y puertos.',
        false, 'Preocupación Menor', 'animalia', 'Larus',
        '{"clase":"Aves","alimentacion":"omnivoro","dieta_detalle":"Peces, invertebrados intermareales, carroña, descartes pesqueros, huevos y polluelos de otras aves; también residuos urbanos.","comportamiento":{"actividad":"diurno","social":"colonial","migratorio":false},"tamano_promedio_cm":58,"peso_promedio_g":1000,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en el suelo de islotes y puntas entre octubre y enero, con dos a tres huevos."}',
        '["IUCN Red List — Larus dominicanus","Aves de Chile — Guía de campo"]'
    ),
    (
        'Haematopus ater', 'Pilpilén negro', 'Vieillot & Oudart, 1825',
        'Ave costera enteramente negra con pico rojo intenso, largo y comprimido lateralmente, patas rosadas y un iris amarillo rodeado de un anillo orbital rojo. El pico es una herramienta especializada: lo introduce entre las valvas de los bivalvos para cortar el músculo aductor, o golpea y desprende lapas y quitones adheridos a la roca. Vive estrictamente asociado a costas rocosas expuestas, donde defiende territorios estables en pareja durante todo el año, y emite un silbido agudo y repetido al ser molestado. Su nido es una simple depresión sobre la roca, lo que lo hace muy vulnerable al pisoteo y a los perros asilvestrados.',
        'Costas rocosas expuestas y plataformas de abrasión con intermareal desarrollado.',
        'Residente en los roqueríos de la costa occidental y en puntas rocosas del mar interior.',
        false, 'Preocupación Menor', 'animalia', 'Haematopus',
        '{"clase":"Aves","alimentacion":"carnivoro","dieta_detalle":"Moluscos del intermareal rocoso —lapas, quitones, mejillones— abiertos o desprendidos con el pico; también poliquetos y crustáceos.","comportamiento":{"actividad":"diurno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":45,"peso_promedio_g":700,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica sobre roca desnuda en primavera, con uno a dos huevos crípticos."}',
        '["IUCN Red List — Haematopus ater","Aves de Chile — Guía de campo"]'
    ),
    (
        'Pteroptochos tarnii', 'Hued-hued del sur', '(P.P. King, 1831)',
        'El mayor de los tapaculos chilenos, de cuerpo rechoncho, patas largas y robustas y cola frecuentemente erguida. El pecho y la garganta son de un castaño rojizo intenso que contrasta con el dorso pardo oscuro y el vientre barrado. Es un ave casi terrestre: vuela poco y mal, y se desplaza corriendo y saltando entre la hojarasca, escarbando con las patas como una gallina en miniatura. Su canto es una serie de notas graves, huecas y repetidas que dan nombre a la especie y que suele responder a la imitación humana. Excava túneles en barrancos de tierra para nidificar, lo que lo liga a bosques con taludes y suelos profundos.',
        'Sotobosque denso y húmedo del bosque templado, con hojarasca profunda, troncos caídos y barrancos de tierra.',
        'Presente en bosques bien conservados de toda la Isla Grande, más escaso en fragmentos pequeños.',
        false, 'Preocupación Menor', 'animalia', 'Pteroptochos',
        '{"clase":"Aves","alimentacion":"insectivoro","dieta_detalle":"Invertebrados del mantillo obtenidos escarbando: coleópteros, larvas, lombrices y arañas; ocasionalmente semillas y frutos caídos.","comportamiento":{"actividad":"diurno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":24,"peso_promedio_g":140,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en primavera en el fondo de túneles excavados en barrancos, con dos a tres huevos blancos."}',
        '["IUCN Red List — Pteroptochos tarnii","Aves de Chile — Guía de campo"]'
    ),
    (
        'Sephanoides sephaniodes', 'Picaflor chico', '(Lesson & Garnot, 1827)',
        'El picaflor más austral del mundo y el único residente permanente del bosque templado lluvioso. De plumaje verde bronceado, el macho luce una corona iridiscente que va del rojo al dorado según el ángulo de la luz. Bate las alas a más de cincuenta ciclos por segundo para sostener el vuelo estacionario con que liba, y afronta las noches frías entrando en torpor: baja su temperatura corporal y su metabolismo para no agotar reservas. Es el polinizador principal de más de veinte especies leñosas del bosque austral —entre ellas el copihue, el notro y el chilco—, varias de las cuales tienen flores tubulares rojas coevolucionadas con su pico.',
        'Interior y borde de bosque templado, matorrales y jardines con flores tubulares; asciende a matorral subandino en verano.',
        'Común en toda la isla durante la primavera y el verano; parte de la población se desplaza al norte en invierno.',
        false, 'Preocupación Menor', 'animalia', 'Sephanoides',
        '{"clase":"Aves","alimentacion":"nectarivoro","dieta_detalle":"Néctar de flores tubulares —copihue, notro, chilco, quintral— complementado con pequeños insectos y arañas capturados al vuelo, fuente indispensable de proteína.","comportamiento":{"actividad":"diurno","social":"solitario","migratorio":true},"tamano_promedio_cm":10,"peso_promedio_g":6,"reproduccion":"oviparo","epoca_reproductiva":"Construye un nido diminuto de musgo y telaraña; pone dos huevos y la hembra cría sola."}',
        '["IUCN Red List — Sephanoides sephaniodes","Aves de Chile — Guía de campo"]'
    ),
    (
        'Enicognathus ferrugineus', 'Cachaña', '(Statius Muller, 1776)',
        'El loro más austral del planeta, de plumaje verde con vermiculaciones oscuras, frente y vientre teñidos de rojo ladrillo y cola larga y puntiaguda del mismo tono. Vuela en bandadas bulliciosas que se detectan mucho antes por el chillido que por la vista, y que se desplazan estacionalmente siguiendo la disponibilidad de semillas. Su alimento clave son las semillas de Nothofagus y de coníferas nativas, que abre con el pico robusto; en años de escasez recurre a brotes, frutos y cultivos, lo que ocasionalmente genera conflicto con la agricultura. Anida en cavidades de árboles viejos, por lo que depende de bosques con árboles maduros y madera muerta en pie.',
        'Bosques maduros de Nothofagus y bosque mixto templado, con árboles de gran diámetro que ofrezcan cavidades.',
        'Presente en bosques del interior de la isla, en bandadas móviles que aparecen y desaparecen según la fructificación.',
        false, 'Preocupación Menor', 'animalia', 'Enicognathus',
        '{"clase":"Aves","alimentacion":"granivoro","dieta_detalle":"Semillas de Nothofagus, piñones y semillas de coníferas nativas, brotes tiernos, frutos y flores; ocasionalmente cultivos.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":34,"peso_promedio_g":140,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en cavidades de árboles viejos entre noviembre y enero, con cuatro a seis huevos."}',
        '["IUCN Red List — Enicognathus ferrugineus","Aves de Chile — Guía de campo"]'
    ),
    (
        'Campephilus magellanicus', 'Carpintero negro', '(King, 1827)',
        'El carpintero más grande de Sudamérica y una de las especies indicadoras del estado del bosque austral. El macho tiene la cabeza entera de un rojo carmesí intenso con una cresta puntiaguda; la hembra es negra con una cresta rizada hacia adelante y solo una mancha roja en la base del pico. Golpea los troncos con un doble tamborileo característico —dos impactos secos y espaciados— que funciona como señal territorial audible a cientos de metros. Excava en madera muerta o en descomposición buscando larvas de coleópteros xilófagos, y las cavidades que abandona son reutilizadas por loros, murciélagos y otras aves, lo que lo convierte en especie clave para la comunidad del bosque.',
        'Bosques maduros de Nothofagus y bosque siempreverde con abundante madera muerta en pie y troncos de gran diámetro.',
        'Presente en los bosques mejor conservados de la Isla Grande; ausente o muy escaso en renovales jóvenes.',
        false, 'Preocupación Menor', 'animalia', 'Campephilus',
        '{"clase":"Aves","alimentacion":"insectivoro","dieta_detalle":"Larvas de coleópteros xilófagos extraídas de madera muerta, además de hormigas, arañas y ocasionalmente huevos y frutos.","comportamiento":{"actividad":"diurno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":40,"peso_promedio_g":330,"reproduccion":"oviparo","epoca_reproductiva":"Excava una cavidad nueva cada temporada en un tronco muerto; pone uno a dos huevos entre octubre y diciembre."}',
        '["IUCN Red List — Campephilus magellanicus","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Strix rufipes', 'Concón', 'King, 1828',
        'Búho mediano de aspecto compacto, sin penachos auriculares, con un disco facial bien delimitado, ojos oscuros y un plumaje densamente barrado en pardo, ocre y blanco que lo camufla de manera casi perfecta contra la corteza. Caza al acecho desde una percha, detectando presas por oído gracias a la disposición asimétrica de sus aberturas auriculares, y vuela en silencio por el flecado aterciopelado del borde de las plumas primarias. Su llamado —dos notas roncas y espaciadas— es uno de los sonidos nocturnos característicos del bosque chilote. Nidifica en cavidades grandes, lo que lo hace dependiente de árboles añosos.',
        'Interior de bosque templado maduro y renovales densos con árboles ahuecados; también bosques de ribera.',
        'Residente en bosques de toda la isla, detectado sobre todo por su canto en noches de calma.',
        false, 'Preocupación Menor', 'animalia', 'Strix',
        '{"clase":"Aves","alimentacion":"carnivoro","dieta_detalle":"Micromamíferos —ratones colilargos y marsupiales pequeños—, aves dormidas, anfibios y grandes insectos.","comportamiento":{"actividad":"nocturno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":37,"peso_promedio_g":350,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en cavidades naturales durante la primavera, con dos a tres huevos blancos."}',
        '["IUCN Red List — Strix rufipes","Aves de Chile — Guía de campo"]'
    ),
    (
        'Glaucidium nana', 'Chuncho', '(King, 1828)',
        'Búho diminuto, apenas mayor que un gorrión, de cabeza redonda finamente moteada y cola barrada que suele mover de lado a lado al posarse. En la nuca presenta dos manchas oscuras rodeadas de claro que simulan un par de ojos: un patrón que desconcierta a los depredadores que atacan por detrás. A diferencia de la mayoría de los búhos es activo de día, sobre todo al amanecer y al atardecer, y caza presas sorprendentemente grandes en relación con su tamaño. Su canto es una serie monótona de notas iguales y repetidas que provoca una reacción inmediata en las aves pequeñas del bosque, que se congregan a acosarlo; esa respuesta de mobbing es la forma más habitual de detectarlo.',
        'Bordes de bosque, renovales, matorral arbolado y arboledas rurales; tolera paisajes fragmentados.',
        'Ampliamente distribuido en la isla, incluso en cercanías de casas y caminos.',
        false, 'Preocupación Menor', 'animalia', 'Glaucidium',
        '{"clase":"Aves","alimentacion":"carnivoro","dieta_detalle":"Insectos grandes, aves pequeñas, roedores y lagartijas; captura presas de hasta su propio peso.","comportamiento":{"actividad":"crepuscular","social":"solitario","migratorio":false},"tamano_promedio_cm":19,"peso_promedio_g":60,"reproduccion":"oviparo","epoca_reproductiva":"Ocupa cavidades abandonadas de carpintero en primavera, con tres a cinco huevos."}',
        '["IUCN Red List — Glaucidium nana","Aves de Chile — Guía de campo"]'
    ),
    (
        'Tyto alba', 'Lechuza blanca', '(Scopoli, 1769)',
        'Rapaz nocturna de distribución casi mundial, inconfundible por su disco facial blanco en forma de corazón, ojos negros pequeños y plumaje dorsal dorado jaspeado sobre partes inferiores muy claras. Su disco facial funciona como una antena parabólica que concentra el sonido hacia oídos asimétricos, permitiéndole localizar y capturar roedores en oscuridad total. Vuela sin ruido audible y emite un chillido áspero en lugar de ulular. Regurgita egagrópilas —bolas compactas de huesos y pelo— cuyo análisis permite reconstruir con precisión la comunidad de micromamíferos de una zona, motivo por el que es una especie muy usada en estudios ecológicos. Nidifica en construcciones, campanarios y galpones, y su presencia es un control biológico gratuito de roedores.',
        'Paisajes abiertos y agrícolas con construcciones, galpones y arboledas donde reposar y nidificar.',
        'Presente en zonas rurales de la isla, asociada a galpones y edificaciones antiguas.',
        false, 'Preocupación Menor', 'animalia', 'Tyto',
        '{"clase":"Aves","alimentacion":"carnivoro","dieta_detalle":"Micromamíferos, principalmente roedores; en menor medida aves pequeñas, murciélagos e insectos. Su dieta se reconstruye analizando egagrópilas.","comportamiento":{"actividad":"nocturno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":35,"peso_promedio_g":350,"reproduccion":"oviparo","epoca_reproductiva":"Puede criar en cualquier época si hay abundancia de roedores; pone de cuatro a siete huevos."}',
        '["IUCN Red List — Tyto alba","Aves de Chile — Guía de campo"]'
    ),
    (
        'Curaeus curaeus', 'Tordo', '(Molina, 1782)',
        'Paseriforme enteramente negro con brillo azulado o verdoso a la luz directa, pico cónico y agudo y cola relativamente larga. Se mueve en bandadas ruidosas que recorren praderas, bordes de bosque y sectores rurales, caminando por el suelo con un balanceo característico. Su canto es un silbido melodioso y aflautado, muy distinto de los chirridos de contacto que emite la bandada. Es marcadamente oportunista: sigue al ganado para capturar insectos levantados por el pisoteo, aprovecha desperdicios y frecuenta comederos. Esa plasticidad lo ha favorecido con la expansión de la pradera ganadera en el archipiélago.',
        'Praderas, bordes de bosque, matorrales y áreas rurales con arboledas.',
        'Muy común en todo el paisaje rural de la isla, en bandadas durante el otoño y el invierno.',
        false, 'Preocupación Menor', 'animalia', 'Curaeus',
        '{"clase":"Aves","alimentacion":"omnivoro","dieta_detalle":"Insectos y larvas del suelo, semillas, frutos, brotes y restos de origen antrópico.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":27,"peso_promedio_g":100,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en primavera en arbustos densos, con tres a cuatro huevos verdosos manchados."}',
        '["IUCN Red List — Curaeus curaeus","Aves de Chile — Guía de campo"]'
    ),
    (
        'Turdus falcklandii', 'Zorzal', 'Quoy & Gaimard, 1824',
        'Ave de tamaño medio, de dorso pardo oliváceo, vientre más claro, cabeza oscura y pico y patas de un amarillo anaranjado vivo que resaltan a distancia. Es uno de los cantores más notables del amanecer austral: emite frases melodiosas y variadas desde perchas expuestas, sobre todo al alba y al atardecer. Forrajea en el suelo con una secuencia inconfundible de carreras cortas y pausas en las que inclina la cabeza para detectar lombrices. Cumple un papel importante como dispersor de semillas del sotobosque, ya que traga enteros los frutos carnosos de murta, chaura, michay y luma, y deposita las semillas viables lejos de la planta madre.',
        'Bosque, borde de bosque, matorral, praderas con arbolado y jardines; muy tolerante a la presencia humana.',
        'Abundante en toda la isla, tanto en bosque nativo como en huertos y pueblos.',
        false, 'Preocupación Menor', 'animalia', 'Turdus',
        '{"clase":"Aves","alimentacion":"omnivoro","dieta_detalle":"Lombrices e invertebrados del suelo durante la crianza, y frutos carnosos del sotobosque —murta, chaura, michay, luma— el resto del año.","comportamiento":{"actividad":"diurno","social":"solitario","migratorio":false},"tamano_promedio_cm":26,"peso_promedio_g":100,"reproduccion":"oviparo","epoca_reproductiva":"Construye un nido de barro y fibras entre septiembre y enero; puede sacar dos nidadas por temporada."}',
        '["IUCN Red List — Turdus falcklandii","Aves de Chile — Guía de campo"]'
    ),
    (
        'Elaenia albiceps', 'Fío-fío', '(d''Orbigny & Lafresnaye, 1837)',
        'Atrapamoscas pequeño y discreto, de plumaje pardo oliváceo, dos barras alares claras y un penacho blanco semioculto en la corona que solo se hace visible cuando el ave se excita. Debe su nombre al canto: un silbido nasal, ascendente y quejumbroso que repite incansablemente y que marca el inicio de la primavera en el bosque austral. Es un migrante de larga distancia: pasa el invierno en el centro y norte del continente y llega a Chiloé en septiembre para reproducirse, retirándose en marzo. Caza insectos al vuelo desde perchas expuestas y, hacia el final del verano, incorpora una proporción alta de frutos, actuando como dispersor.',
        'Borde de bosque, matorral, renovales y arboledas abiertas; evita el interior más sombrío del bosque cerrado.',
        'Migrante estival abundante en toda la isla entre septiembre y marzo.',
        false, 'Preocupación Menor', 'animalia', 'Elaenia',
        '{"clase":"Aves","alimentacion":"insectivoro","dieta_detalle":"Insectos capturados al vuelo o espigados del follaje; en verano y otoño complementa con una proporción importante de frutos pequeños.","comportamiento":{"actividad":"diurno","social":"en_pareja","migratorio":true},"tamano_promedio_cm":15,"peso_promedio_g":16,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica entre noviembre y enero en una taza pequeña sobre horqueta, con dos a tres huevos."}',
        '["IUCN Red List — Elaenia albiceps","Aves de Chile — Guía de campo"]'
    ),
    (
        'Aphrastura spinicauda', 'Rayadito', '(Gmelin, 1789)',
        'Furnárido diminuto, inquieto y confiado, de cabeza rayada en negro y ocre, ceja amarillenta muy marcada y una cola escalonada cuyas plumas terminan en raquis desnudos y rígidos que usa como apoyo al trepar. Recorre troncos y ramas en todas las posiciones, incluso boca abajo, explorando grietas de corteza con movimientos rápidos y continuos. Se desplaza en grupos familiares bulliciosos que suelen formar el núcleo de bandadas mixtas del bosque templado, arrastrando consigo a otras especies insectívoras. Nidifica en cavidades, y por eso responde tan bien a la instalación de cajas anidaderas, lo que lo ha convertido en el modelo más estudiado de la ecología de aves del bosque austral.',
        'Interior y borde de bosque templado, renovales y matorral arbolado; requiere cavidades para nidificar.',
        'Una de las aves más abundantes y fáciles de ver en cualquier bosque de la isla.',
        false, 'Preocupación Menor', 'animalia', 'Aphrastura',
        '{"clase":"Aves","alimentacion":"insectivoro","dieta_detalle":"Artrópodos pequeños extraídos de grietas de corteza, musgos y follaje: coleópteros, arañas, larvas y huevos de insecto.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":14,"peso_promedio_g":11,"reproduccion":"oviparo","epoca_reproductiva":"Ocupa cavidades y cajas anidaderas entre octubre y diciembre, con cuatro a cinco huevos."}',
        '["IUCN Red List — Aphrastura spinicauda","Aves de Chile — Guía de campo"]'
    ),
    (
        'Sylviorthorhynchus desmursii', 'Colilarga', 'Des Murs, 1847',
        'Ave minúscula de cuerpo pardo rojizo con una cola desproporcionada, formada por unas pocas plumas filamentosas que pueden triplicar la longitud del cuerpo y que arrastra o levanta al moverse entre la vegetación. Es extremadamente difícil de observar: vive en el interior de matorrales impenetrables de quila y en la maraña del sotobosque, donde se desplaza a saltos cortos y rara vez cruza espacios abiertos. Se detecta casi siempre por su canto, un trino agudo y seco emitido desde el interior de la espesura. Su dependencia estricta del sotobosque denso la hace sensible a la ganadería y al ramoneo, que abren y aclaran el estrato bajo del bosque.',
        'Sotobosque muy denso, matorrales de quila, murtillares y vegetación ribereña enmarañada.',
        'Presente en bosques con sotobosque bien conservado; escasa donde el ganado ha abierto el estrato bajo.',
        false, 'Preocupación Menor', 'animalia', 'Sylviorthorhynchus',
        '{"clase":"Aves","alimentacion":"insectivoro","dieta_detalle":"Artrópodos pequeños capturados en el interior de la maraña vegetal: arañas, larvas y microcoleópteros.","comportamiento":{"actividad":"diurno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":24,"peso_promedio_g":10,"reproduccion":"oviparo","epoca_reproductiva":"Construye un nido esférico cerrado dentro de la espesura, en primavera, con dos a tres huevos."}',
        '["IUCN Red List — Sylviorthorhynchus desmursii","Aves de Chile — Guía de campo"]'
    ),
    (
        'Phrygilus patagonicus', 'Cometocino patagónico', 'Lowe, 1923',
        'Ave granívora de aspecto vistoso: el macho tiene cabeza y garganta gris pizarra, dorso verde oliva, pecho y flancos de un naranja amarillento intenso y rabadilla verdosa; la hembra repite el patrón en tonos apagados. El pico es cónico y robusto, adaptado a partir semillas duras. Se mueve en parejas o grupos pequeños por bordes de bosque, claros y matorrales, bajando al suelo a alimentarse y regresando rápidamente a la cobertura. En invierno desciende desde las áreas más altas y se acerca a sectores habitados, donde se le ve con frecuencia en huertos y comederos.',
        'Bordes de bosque, claros, matorrales y áreas rurales arboladas; en verano también matorral de altura.',
        'Común en toda la isla, más conspicuo en otoño e invierno cerca de casas y huertos.',
        false, 'Preocupación Menor', 'animalia', 'Phrygilus',
        '{"clase":"Aves","alimentacion":"granivoro","dieta_detalle":"Semillas de gramíneas y herbáceas, brotes y frutos pequeños; durante la crianza incorpora insectos para los pollos.","comportamiento":{"actividad":"diurno","social":"en_pareja","migratorio":false},"tamano_promedio_cm":16,"peso_promedio_g":25,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica en primavera en arbustos densos, con tres a cuatro huevos."}',
        '["IUCN Red List — Phrygilus patagonicus","Aves de Chile — Guía de campo"]'
    ),
    (
        'Milvago chimango', 'Tiuque', '(Vieillot, 1816)',
        'Falcónido pardo de tamaño mediano, patas amarillentas y una mancha clara en la base de las primarias visible en vuelo. A diferencia de los halcones cazadores, es un generalista terrestre que camina por el suelo, sigue tractores y arados para capturar invertebrados expuestos, hurga en basurales y aprovecha carroña. Es ruidoso y confiado, y se agrupa en concentraciones grandes donde hay alimento predecible. Su éxito está directamente ligado a la transformación del paisaje: la ganadería, la agricultura y los desechos urbanos le han abierto un nicho que explota mejor que casi cualquier otra rapaz del sur de Chile.',
        'Praderas, campos arados, humedales, caletas, vertederos y áreas urbanas.',
        'Muy común en todo el paisaje abierto y rural de la isla, y en caletas pesqueras.',
        false, 'Preocupación Menor', 'animalia', 'Milvago',
        '{"clase":"Aves","alimentacion":"omnivoro","dieta_detalle":"Invertebrados del suelo, carroña, descartes de pesca, huevos y polluelos, roedores y residuos orgánicos.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":40,"peso_promedio_g":300,"reproduccion":"oviparo","epoca_reproductiva":"Nidifica entre octubre y diciembre en árboles o matorrales, con dos a tres huevos."}',
        '["IUCN Red List — Milvago chimango","Aves de Chile — Guía de campo"]'
    ),
    (
        'Lontra felina', 'Chungungo', '(Molina, 1782)',
        'La nutria marina más pequeña del mundo y la única del género que vive de manera casi exclusiva en el mar. De pelaje pardo oscuro, denso y aislante —carece de capa de grasa subcutánea, de modo que depende del aire atrapado entre los pelos para no perder calor—, tiene cuerpo alargado, cola gruesa y membranas interdigitales. Vive en costas rocosas expuestas, donde descansa en cuevas y grietas por encima de la línea de marea y bucea en la rompiente en inmersiones cortas. Su dependencia de un hábitat lineal y estrecho, sumada a la caza histórica por su piel y a los conflictos con la pesca artesanal, mantiene sus poblaciones fragmentadas y escasas.',
        'Costas rocosas expuestas con cuevas y grietas sobre la línea de marea alta; bosques de huiro adyacentes.',
        'Presente en la costa occidental expuesta y en roqueríos del mar interior, en densidades bajas.',
        false, 'En Peligro', 'animalia', 'Lontra',
        '{"clase":"Mammalia","alimentacion":"carnivoro","dieta_detalle":"Crustáceos —jaibas y camarones de roca—, peces de roquerío y moluscos, capturados en buceos de menos de un minuto y consumidos en la superficie o en tierra.","comportamiento":{"actividad":"crepuscular","social":"solitario","migratorio":false},"tamano_promedio_cm":100,"peso_promedio_g":4500,"reproduccion":"viviparo","epoca_reproductiva":"Pare en cuevas costeras entre diciembre y enero, con dos crías por camada."}',
        '["IUCN Red List — Lontra felina","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Lontra provocax', 'Huillín', 'Thomas, 1908',
        'Nutria de agua dulce y salobre, mayor y más robusta que el chungungo, con pelaje pardo oscuro, garganta más clara y una cola gruesa que se afina hacia la punta. En el sur del país ocupa también ambientes marinos protegidos, donde usa el bosque de huiro y las orillas con vegetación densa como cobertura. Es muy sensible a la alteración de las riberas: necesita orillas con raíces, troncos caídos y vegetación colgante para refugiarse y para excavar madrigueras. La canalización de ríos, la pérdida de vegetación ribereña, la contaminación y la caza histórica la han reducido a poblaciones fragmentadas, y su presencia se usa como indicador de la calidad de los sistemas ribereños.',
        'Ríos, lagos y estuarios con vegetación ribereña densa; en el sur también canales marinos protegidos con bosque de huiro.',
        'Registrado en ríos, estuarios y canales interiores del archipiélago, en poblaciones dispersas.',
        false, 'En Peligro', 'animalia', 'Lontra',
        '{"clase":"Mammalia","alimentacion":"carnivoro","dieta_detalle":"Crustáceos de agua dulce, peces y moluscos; en ambientes marinos incorpora jaibas y peces de roquerío.","comportamiento":{"actividad":"crepuscular","social":"solitario","migratorio":false},"tamano_promedio_cm":115,"peso_promedio_g":7000,"reproduccion":"viviparo","epoca_reproductiva":"Camadas de una a tres crías, paridas en madrigueras excavadas en la ribera."}',
        '["IUCN Red List — Lontra provocax","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Puma concolor', 'Puma', '(Linnaeus, 1771)',
        'El mayor depredador terrestre de Chiloé y el felino de distribución más amplia del continente americano. De pelaje uniforme leonado, cabeza pequeña en proporción al cuerpo, cola larga y musculatura posterior potente que le permite saltos de varios metros. Es un cazador de acecho y emboscada que depende de la cobertura vegetal para aproximarse a la presa. En el archipiélago su dieta está dominada por el pudú, y su presencia regula indirectamente el ramoneo del sotobosque. Es esquivo y de hábitos crepusculares y nocturnos, por lo que la mayoría de los registros provienen de huellas, fecas y cámaras trampa más que de observación directa. El conflicto con la ganadería, cuando ataca ovinos, sigue siendo su principal amenaza local.',
        'Bosque templado denso y renovales con buena cobertura; también matorrales y bordes de pradera durante la caza.',
        'Presente en los sectores boscosos menos intervenidos de la Isla Grande, en densidades bajas.',
        false, 'Preocupación Menor', 'animalia', 'Puma',
        '{"clase":"Mammalia","alimentacion":"carnivoro","dieta_detalle":"Pudú como presa principal en la isla, complementado con coipos, roedores, aves y ganado ovino doméstico.","comportamiento":{"actividad":"crepuscular","social":"solitario","migratorio":false},"tamano_promedio_cm":180,"peso_promedio_g":50000,"reproduccion":"viviparo","epoca_reproductiva":"Sin estacionalidad marcada; camadas de dos a tres cachorros con manchas al nacer."}',
        '["IUCN Red List — Puma concolor","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Leopardus guigna', 'Güiña', '(Molina, 1782)',
        'El felino más pequeño de América, del tamaño de un gato doméstico pequeño pero de aspecto más compacto, con cola gruesa y anillada, pelaje ocráceo cubierto de manchas negras redondeadas y orejas con mancha blanca posterior. Es un excelente trepador que caza tanto en el suelo como en el estrato arbóreo, y su tamaño reducido le permite moverse dentro de la maraña del sotobosque. Está estrictamente ligada al bosque templado: soporta cierta fragmentación si se conservan corredores de vegetación, pero desaparece en paisajes completamente abiertos. Las amenazas principales son la pérdida de hábitat, los perros asilvestrados, el atropello y la persecución por depredar aves de corral, además de las enfermedades transmitidas por gatos domésticos.',
        'Bosque templado lluvioso, renovales y matorral denso; usa cercos vivos y corredores de vegetación en paisajes fragmentados.',
        'Presente en toda la Isla Grande, incluso cerca de asentamientos rurales donde queda cobertura arbustiva.',
        false, 'Vulnerable', 'animalia', 'Leopardus',
        '{"clase":"Mammalia","alimentacion":"carnivoro","dieta_detalle":"Roedores, marsupiales pequeños, aves y sus huevos, lagartijas e insectos grandes; ocasionalmente aves de corral.","comportamiento":{"actividad":"nocturno","social":"solitario","migratorio":false},"tamano_promedio_cm":70,"peso_promedio_g":2200,"reproduccion":"viviparo","epoca_reproductiva":"Camadas de una a tres crías, nacidas en huecos de árbol o entre raíces."}',
        '["IUCN Red List — Leopardus guigna","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Conepatus chinga', 'Chingue', '(Molina, 1782)',
        'Mefítido inconfundible por su patrón aposemático: pelaje negro con dos bandas blancas anchas que recorren el dorso desde la cabeza hasta la cola, peluda y también blanca. Ese diseño de alto contraste es una advertencia honesta: ante una amenaza levanta la cola, patea el suelo y, si el aviso no basta, rocía una secreción de las glándulas anales cuyo olor persiste durante días y es irritante para las mucosas. Tiene hocico alargado y garras delanteras robustas con las que escarba el suelo buscando larvas, y su vista es pobre en comparación con su olfato. Es nocturno, solitario y de movimientos lentos y confiados, lo que lamentablemente lo convierte en víctima frecuente de atropellos.',
        'Praderas, matorrales, bordes de bosque y áreas rurales con suelo blando donde escarbar.',
        'Presente en zonas rurales y de matorral de la isla; se detecta a menudo por su olor o por atropellos en caminos.',
        false, 'Preocupación Menor', 'animalia', 'Conepatus',
        '{"clase":"Mammalia","alimentacion":"omnivoro","dieta_detalle":"Larvas de coleópteros y otros invertebrados excavados del suelo, complementados con frutos, huevos, carroña y pequeños vertebrados.","comportamiento":{"actividad":"nocturno","social":"solitario","migratorio":false},"tamano_promedio_cm":60,"peso_promedio_g":2000,"reproduccion":"viviparo","epoca_reproductiva":"Pare en primavera en madrigueras o huecos, con dos a cinco crías."}',
        '["IUCN Red List — Conepatus chinga","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Myocastor coypus', 'Coipo', '(Molina, 1782)',
        'Roedor semiacuático de gran tamaño, con pelaje pardo denso, incisivos de un naranja intenso muy visible, orejas pequeñas, cola larga, cilíndrica y casi desnuda, y membranas interdigitales en las patas traseras. Las mamas de la hembra están dispuestas lateralmente en el dorso, adaptación que permite a las crías amamantarse mientras la madre nada. Construye plataformas y madrigueras en las riberas y consume grandes cantidades de vegetación acuática, con lo que modela la estructura de los humedales. Fue intensamente cazado por su piel y llevado a criaderos en varios continentes, desde donde escapó y se volvió invasor; en su área nativa, en cambio, sus poblaciones dependen de la conservación de las riberas.',
        'Humedales, ríos de curso lento, lagunas, turberas y canales con vegetación palustre abundante.',
        'Presente en humedales, lagunas y cursos de agua lentos de la isla, especialmente en el sector oriental.',
        false, 'Preocupación Menor', 'animalia', 'Myocastor',
        '{"clase":"Mammalia","alimentacion":"herbivoro","dieta_detalle":"Tallos, rizomas y hojas de plantas palustres y acuáticas; ocasionalmente cultivos ribereños y, de forma marginal, moluscos de agua dulce.","comportamiento":{"actividad":"crepuscular","social":"gregario","migratorio":false},"tamano_promedio_cm":90,"peso_promedio_g":7000,"reproduccion":"viviparo","epoca_reproductiva":"Se reproduce todo el año; camadas de cuatro a seis crías nacidas ya con pelo y ojos abiertos."}',
        '["IUCN Red List — Myocastor coypus","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Oligoryzomys longicaudatus', 'Ratón colilargo', '(Bennett, 1832)',
        'Roedor pequeño de pelaje ocráceo a pardo amarillento, vientre claro y una cola que supera con holgura la longitud del cuerpo, adaptación al desplazamiento entre la vegetación densa donde trepa con facilidad. Es una especie clave del bosque templado: constituye la presa base de rapaces nocturnas, felinos pequeños y culebras. Su población fluctúa de manera drástica tras la floración masiva y sincronizada de la quila, fenómeno que ocurre cada varias décadas y libera una cantidad enorme de semilla; el aumento explosivo que sigue se conoce localmente como ratada. Es el reservorio del hantavirus Andes en Chile, transmitido por inhalación de aerosoles de sus excretas, motivo por el cual las viviendas y bodegas rurales cerradas deben ventilarse antes de limpiarse.',
        'Sotobosque denso, matorrales de quila, bordes de bosque, praderas con cobertura y construcciones rurales.',
        'Ampliamente distribuido en toda la isla, tanto en bosque como en el entorno de viviendas rurales.',
        false, 'Preocupación Menor', 'animalia', 'Oligoryzomys',
        '{"clase":"Mammalia","alimentacion":"omnivoro","dieta_detalle":"Semillas —sobre todo de quila—, frutos, hongos e insectos; su abundancia sigue los ciclos de fructificación de la quila.","comportamiento":{"actividad":"nocturno","social":"solitario","migratorio":false},"tamano_promedio_cm":24,"peso_promedio_g":28,"reproduccion":"viviparo","epoca_reproductiva":"Varias camadas por temporada entre primavera y otoño, con tres a seis crías cada una."}',
        '["IUCN Red List — Oligoryzomys longicaudatus","Instituto de Salud Pública de Chile — Vigilancia de hantavirus"]'
    ),
    (
        'Otaria flavescens', 'Lobo marino común', '(Shaw, 1800)',
        'Pinnípedo de marcado dimorfismo sexual: el macho adulto puede superar los trescientos kilos y desarrolla una melena gruesa en el cuello y una cabeza ancha de hocico respingado, mientras que la hembra es mucho menor y de color más claro. Como todos los otáridos conserva orejas externas y puede girar las aletas traseras hacia adelante, lo que le permite desplazarse con relativa agilidad en tierra. Se reproduce en loberas tradicionales donde los machos establecen territorios y defienden harenes durante el verano. En el archipiélago es una presencia constante en caletas y balsas de cultivo, donde el conflicto con la pesca artesanal y la salmonicultura —por depredación y daño a las redes— es un asunto recurrente.',
        'Roqueríos costeros e islotes para descanso y reproducción; forrajea en aguas costeras y de plataforma.',
        'Abundante en roqueríos, islotes y caletas de todo el archipiélago, con loberas establecidas.',
        false, 'Preocupación Menor', 'animalia', 'Otaria',
        '{"clase":"Mammalia","alimentacion":"piscivoro","dieta_detalle":"Peces demersales y pelágicos, cefalópodos y crustáceos; aprovecha además descartes y peces escapados de balsas de cultivo.","comportamiento":{"actividad":"diurno","social":"colonial","migratorio":false},"tamano_promedio_cm":250,"peso_promedio_g":300000,"reproduccion":"viviparo","epoca_reproductiva":"Paridera y cópula entre diciembre y febrero en loberas tradicionales; una cría por hembra al año."}',
        '["IUCN Red List — Otaria flavescens","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Eubalaena australis', 'Ballena franca austral', '(Desmoulins, 1822)',
        'Misticeto robusto y sin aleta dorsal, reconocible por las callosidades de la cabeza —engrosamientos de piel colonizados por crustáceos ciámidos— cuyo patrón es único en cada individuo y permite la fotoidentificación. Su nombre proviene de la época ballenera: era la ballena correcta que cazar, porque nada lento, se acerca a la costa y flota al morir. Esa combinación la llevó al borde de la extinción, y la población chileno-peruana sigue siendo una de las más críticas del planeta, con muy pocos individuos avistados por año. Filtra copépodos y krill nadando con la boca abierta a través de barbas largas y finas, y sus avistamientos en aguas del archipiélago son excepcionales y de alto valor científico.',
        'Aguas costeras y bahías protegidas para reproducción; áreas de alimentación en latitudes altas.',
        'Avistamientos ocasionales y excepcionales en aguas interiores y en el golfo, cada uno de interés científico.',
        false, 'En Peligro Crítico', 'animalia', 'Eubalaena',
        '{"clase":"Mammalia","alimentacion":"filtrador","dieta_detalle":"Copépodos y krill filtrados mediante nado continuo con la boca abierta a través de barbas de más de dos metros.","comportamiento":{"actividad":"catemeral","social":"solitario","migratorio":true},"tamano_promedio_cm":1500,"peso_promedio_g":40000000,"reproduccion":"viviparo","epoca_reproductiva":"Una cría cada tres años, parida en invierno en aguas costeras templadas."}',
        '["IUCN Red List — Eubalaena australis","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Balaenoptera musculus', 'Ballena azul', '(Linnaeus, 1758)',
        'El animal más grande que ha existido: supera los veinticinco metros y las cien toneladas, y su corazón alcanza el tamaño de un automóvil pequeño. De cuerpo alargado y azul grisáceo moteado, tiene una aleta dorsal diminuta situada muy atrás y surcos ventrales que se expanden enormemente al engullir agua. Emite vocalizaciones de muy baja frecuencia que viajan cientos de kilómetros bajo el agua. El golfo de Corcovado y las aguas del mar interior de Chiloé constituyen una de las áreas de alimentación y crianza más importantes del Pacífico sur oriental, descubierta recién a comienzos de los años dos mil, lo que dio origen a medidas de protección y a la discusión sobre el tráfico marítimo y el ruido submarino en la zona.',
        'Aguas costeras y de plataforma altamente productivas donde se concentra el krill; se desplaza a aguas cálidas en invierno.',
        'El golfo de Corcovado, al sureste del archipiélago, es un área de alimentación y crianza reconocida internacionalmente.',
        false, 'En Peligro', 'animalia', 'Balaenoptera',
        '{"clase":"Mammalia","alimentacion":"filtrador","dieta_detalle":"Krill casi exclusivamente, capturado en embestidas de engullimiento que pueden incorporar un volumen de agua mayor que el propio cuerpo.","comportamiento":{"actividad":"catemeral","social":"solitario","migratorio":true},"tamano_promedio_cm":2500,"peso_promedio_g":120000000,"reproduccion":"viviparo","epoca_reproductiva":"Una cría cada dos o tres años tras casi un año de gestación."}',
        '["IUCN Red List — Balaenoptera musculus","Centro de Conservación Cetacea — Corcovado"]'
    ),
    (
        'Cephalorhynchus eutropia', 'Delfín chileno', '(Gray, 1846)',
        'El único cetáceo endémico de Chile y uno de los delfines más pequeños del mundo: apenas supera el metro y medio. De cuerpo compacto y robusto, cabeza roma sin pico diferenciado y aleta dorsal redondeada, presenta un patrón gris oscuro con vientre blanco y marcas claras detrás de las aletas. Es discreto y esquivo, rara vez salta y casi nunca se acerca a las embarcaciones, a diferencia de otros delfines. Vive en grupos pequeños, de dos a diez individuos, muy fieles a áreas costeras someras, bocas de estuario y canales, lo que lo expone directamente a las redes de enmalle, al tráfico de embarcaciones y a la expansión de la acuicultura en las aguas interiores del archipiélago.',
        'Aguas costeras someras, bocas de río, estuarios y canales con fuerte influencia mareal.',
        'Presente en aguas interiores y bocas de estuario del archipiélago, en grupos pequeños y residentes.',
        true, 'Casi Amenazado', 'animalia', 'Cephalorhynchus',
        '{"clase":"Mammalia","alimentacion":"piscivoro","dieta_detalle":"Peces costeros pequeños como sardina y pejerrey, además de cefalópodos y crustáceos capturados en aguas someras.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":160,"peso_promedio_g":57000,"reproduccion":"viviparo","epoca_reproductiva":"Una cría por parto, con nacimientos concentrados en primavera y verano."}',
        '["IUCN Red List — Cephalorhynchus eutropia","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Lagenorhynchus australis', 'Delfín austral', '(Peale, 1848)',
        'Delfín de tamaño medio y patrón de coloración muy contrastado: dorso negro, flancos grises y vientre blanco, con una banda clara que asciende hacia la aleta dorsal falcada y una característica mancha blanca en la garganta. Es más demostrativo que el delfín chileno: salta, hace surf en la estela de las embarcaciones y forma grupos de mayor tamaño. Habita aguas frías del extremo sur de Sudamérica, con frecuencia asociado a bosques de huiro y a canales con fuerte corriente donde el alimento se concentra. Su captura incidental en redes y su uso histórico como carnada para la pesquería de centolla en el extremo austral fueron amenazas documentadas y hoy prohibidas.',
        'Canales, fiordos y aguas costeras frías, frecuentemente asociado a bosques de huiro y zonas de corriente.',
        'Avistado en canales y aguas expuestas del sur del archipiélago, en grupos de varios individuos.',
        false, 'Preocupación Menor', 'animalia', 'Lagenorhynchus',
        '{"clase":"Mammalia","alimentacion":"piscivoro","dieta_detalle":"Peces pelágicos de cardumen, cefalópodos y crustáceos; caza de forma cooperativa concentrando a las presas.","comportamiento":{"actividad":"diurno","social":"gregario","migratorio":false},"tamano_promedio_cm":210,"peso_promedio_g":115000,"reproduccion":"viviparo","epoca_reproductiva":"Una cría por parto; nacimientos registrados principalmente en verano austral."}',
        '["IUCN Red List — Lagenorhynchus australis","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Myotis chiloensis', 'Murciélago oreja de ratón del sur', '(Waterhouse, 1840)',
        'Microquiróptero pequeño, de pelaje pardo rojizo, orejas cortas y redondeadas y membranas alares desnudas, cuyo nombre específico remite justamente al archipiélago donde fue descrito. Caza insectos al vuelo mediante ecolocalización, emitiendo pulsos ultrasónicos y reconstruyendo el entorno con el eco, lo que le permite maniobrar en la oscuridad total entre el follaje. Es el murciélago más austral del mundo y afronta el invierno con periodos de letargo en refugios protegidos: huecos de árbol, entretechos y galpones. Presta un servicio ecosistémico considerable al consumir cada noche una masa importante de insectos, incluidos mosquitos y polillas de interés agrícola.',
        'Bosques, bordes y áreas rurales; refugios diurnos en huecos de árboles, entretechos y construcciones.',
        'Ampliamente distribuido en la isla, detectable al atardecer sobre claros, cursos de agua y luminarias.',
        false, 'Preocupación Menor', 'animalia', 'Myotis',
        '{"clase":"Mammalia","alimentacion":"insectivoro","dieta_detalle":"Insectos voladores capturados en vuelo mediante ecolocalización: polillas, dípteros, coleópteros pequeños y efemerópteros.","comportamiento":{"actividad":"nocturno","social":"colonial","migratorio":false},"tamano_promedio_cm":9,"peso_promedio_g":8,"reproduccion":"viviparo","epoca_reproductiva":"Una cría por hembra en verano, en colonias de maternidad; entra en letargo invernal."}',
        '["IUCN Red List — Myotis chiloensis","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Rhinoderma darwinii', 'Ranita de Darwin', 'Duméril & Bibron, 1841',
        'Anfibio diminuto y críptico, de color verde o pardo según el individuo, con un apéndice carnoso y puntiagudo en la punta del hocico que, sumado a la postura y a la coloración, lo hace prácticamente indistinguible de una hoja seca del suelo del bosque. Su rasgo más extraordinario es la neomelia: el macho traga los renacuajos y los aloja en su saco vocal, donde completan la metamorfosis alimentándose de secreciones, hasta salir como ranitas formadas por la boca del padre. No existe otro vertebrado con esta estrategia. La especie está en fuerte declive por la pérdida y sustitución del bosque nativo y por la quitridiomicosis, la enfermedad fúngica responsable del colapso de anfibios en todo el mundo; su congénere del norte se considera probablemente extinta.',
        'Suelo del bosque templado húmedo, entre hojarasca, musgos y helechos, con humedad ambiental constante.',
        'Presente en bosques nativos bien conservados de la Isla Grande, en poblaciones localizadas y sensibles.',
        false, 'En Peligro', 'animalia', 'Rhinoderma',
        '{"clase":"Amphibia","alimentacion":"insectivoro","dieta_detalle":"Artrópodos muy pequeños del mantillo: colémbolos, ácaros, arañas diminutas y larvas de insecto.","comportamiento":{"actividad":"diurno","social":"solitario","migratorio":false},"tamano_promedio_cm":3,"peso_promedio_g":2,"reproduccion":"oviparo","epoca_reproductiva":"Postura en el suelo húmedo en primavera; el macho incuba los renacuajos en el saco vocal durante unas seis semanas."}',
        '["IUCN Red List — Rhinoderma darwinii","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Batrachyla taeniata', 'Sapito de antifaz', '(Girard, 1855)',
        'Anuro pequeño y esbelto, de dorso pardo con tonos variables y una banda oscura muy marcada que atraviesa el ojo desde el hocico hasta el tímpano, el antifaz que le da nombre. Tiene hábitos terrestres y se mueve entre la hojarasca húmeda del bosque, lejos del agua durante buena parte del año. Su reproducción es notable: deposita los huevos en depresiones húmedas del suelo o bajo troncos, fuera del agua, y los embriones se desarrollan allí hasta que la subida del nivel de agua por las lluvias otoñales inunda la puesta y libera a los renacuajos. El canto de los machos, una serie de notas cortas y repetidas, se escucha desde el suelo del bosque en las noches de otoño.',
        'Hojarasca y suelo húmedo del bosque templado, cerca de charcas temporales, turberas y cursos de agua estacionales.',
        'Presente en bosques y turberas de la isla, más audible que visible durante el otoño.',
        false, 'Preocupación Menor', 'animalia', 'Batrachyla',
        '{"clase":"Amphibia","alimentacion":"insectivoro","dieta_detalle":"Pequeños artrópodos del mantillo: coleópteros, arañas, colémbolos y larvas.","comportamiento":{"actividad":"nocturno","social":"solitario","migratorio":false},"tamano_promedio_cm":4,"peso_promedio_g":3,"reproduccion":"oviparo","epoca_reproductiva":"Deposita los huevos fuera del agua en otoño; la eclosión se dispara cuando las lluvias inundan la puesta."}',
        '["IUCN Red List — Batrachyla taeniata","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Eupsophus calcaratus', 'Sapito de cuatro ojos del sur', '(Günther, 1881)',
        'Anfibio pequeño del suelo del bosque austral, de dorso pardo o rojizo con manchas irregulares y un vientre marmóreo. Debe su nombre común a dos manchas glandulares oscuras y conspicuas situadas en la región inguinal que, cuando el animal levanta la parte posterior del cuerpo ante una amenaza, simulan un segundo par de ojos de un animal mayor. Se reproduce en pequeñas cavidades húmedas excavadas junto a cursos de agua o en el interior de troncos podridos, donde el macho canta desde el fondo de la cámara; los renacuajos se desarrollan en ese espacio confinado sin alimentarse, sustentados por el vitelo. Es sensible a la desecación y a la sustitución del bosque nativo por plantaciones.',
        'Suelo y hojarasca del bosque templado húmedo, cerca de vertientes, esteros y troncos en descomposición.',
        'Presente en bosques húmedos de la isla, con canto audible desde cavidades del suelo en primavera.',
        false, 'Preocupación Menor', 'animalia', 'Eupsophus',
        '{"clase":"Amphibia","alimentacion":"insectivoro","dieta_detalle":"Artrópodos pequeños de la hojarasca: ácaros, colémbolos, arañas y larvas de insecto.","comportamiento":{"actividad":"nocturno","social":"solitario","migratorio":false},"tamano_promedio_cm":4,"peso_promedio_g":4,"reproduccion":"oviparo","epoca_reproductiva":"Postura en cámaras húmedas subterráneas durante la primavera; los renacuajos no se alimentan y viven del vitelo."}',
        '["IUCN Red List — Eupsophus calcaratus","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Liolaemus pictus', 'Lagartija pintada', '(Duméril & Bibron, 1837)',
        'Lagartija de tamaño mediano y coloración muy variable —de ahí su nombre—, con dorso pardo, verdoso o cobrizo cruzado por bandas y manchas claras y oscuras que varían entre poblaciones. Es la especie de reptil más asociada al bosque templado lluvioso, un ambiente atípico para un grupo mayoritariamente ligado a zonas áridas. Trepa con soltura por troncos, cercos y quilantales, y se asolea en claros y en la madera muerta expuesta durante las horas centrales de días despejados. Es vivípara, una adaptación al clima frío y lluvioso que permite a la hembra regular la temperatura de los embriones desplazándose entre sol y sombra en vez de depender de un nido enterrado.',
        'Claros de bosque, bordes, troncos caídos, cercos de madera y matorral soleado del bosque templado.',
        'Común en bordes de bosque, cercos y leñeras de toda la isla, activa en días soleados.',
        false, 'Preocupación Menor', 'animalia', 'Liolaemus',
        '{"clase":"Reptilia","alimentacion":"insectivoro","dieta_detalle":"Insectos y arañas capturados al acecho; incorpora algunos frutos pequeños y flores de manera ocasional.","comportamiento":{"actividad":"diurno","social":"solitario","migratorio":false},"tamano_promedio_cm":18,"peso_promedio_g":12,"reproduccion":"viviparo","epoca_reproductiva":"Pare de dos a seis crías vivas a fines del verano, tras una gestación que aprovecha la termorregulación materna."}',
        '["IUCN Red List — Liolaemus pictus","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Tachymenis chilensis', 'Culebra de cola corta', '(Schlegel, 1837)',
        'Serpiente de tamaño pequeño a mediano, de cuerpo robusto, cabeza poco diferenciada del cuello y cola notablemente corta, con dorso pardo o grisáceo recorrido por líneas longitudinales oscuras. Es opistoglifa: tiene los dientes inoculadores en la parte posterior del maxilar, de modo que solo puede introducir su secreción con una mordida sostenida y profunda. Sus efectos en humanos son locales y leves —dolor, hinchazón y enrojecimiento—, y no se conocen casos graves en Chile, pero conviene no manipularla. Es de hábitos diurnos y terrestres, se asolea en claros, pedreros y bordes de camino, y ante una amenaza aplana el cuerpo y se retira; muerde solo si se la sujeta.',
        'Claros, bordes de bosque, matorrales, pedreros y zonas rurales con cobertura y sitios de asoleo.',
        'Presente en la isla pero poco frecuente de observar; aparece en bordes de camino y sectores despejados.',
        false, 'Preocupación Menor', 'animalia', 'Tachymenis',
        '{"clase":"Reptilia","alimentacion":"carnivoro","dieta_detalle":"Anfibios y lagartijas principalmente, además de roedores muy pequeños e invertebrados grandes.","comportamiento":{"actividad":"diurno","social":"solitario","migratorio":false},"tamano_promedio_cm":60,"peso_promedio_g":80,"reproduccion":"viviparo","epoca_reproductiva":"Pare crías vivas a fines del verano, adaptación al clima frío del sur."}',
        '["IUCN Red List — Tachymenis chilensis","Ministerio del Medio Ambiente de Chile — Inventario Nacional de Especies"]'
    ),
    (
        'Concholepas concholepas', 'Loco', '(Bruguière, 1789)',
        'Gasterópodo marino de concha gruesa, ovalada y fuertemente esculpida con costillas radiales, cuyo enorme pie musculoso ocupa casi toda la abertura y le permite adherirse a la roca con una fuerza considerable. Pese a su aspecto de lapa, es un múrice depredador: perfora o fuerza las valvas de sus presas y se alimenta principalmente de mitílidos y picorocos, ejerciendo un control descendente sobre la estructura de la comunidad del intermareal rocoso. Su valor comercial provocó una sobreexplotación severa desde los años setenta, que llevó a la veda extractiva y al establecimiento de las áreas de manejo, uno de los experimentos de cogestión pesquera más estudiados del mundo, con resultados especialmente relevantes en las caletas del archipiélago.',
        'Intermareal y submareal rocoso somero, sobre bancos de mitílidos y picorocos, hasta unos cuarenta metros de profundidad.',
        'Presente en toda la costa rocosa del archipiélago; base de numerosas áreas de manejo de las caletas locales.',
        false, 'No Evaluada', 'animalia', 'Concholepas',
        '{"clase":"Gastropoda","alimentacion":"carnivoro","dieta_detalle":"Mitílidos, picorocos y otros invertebrados sésiles, abiertos mediante presión del pie y acción de la rádula.","comportamiento":{"actividad":"nocturno","social":"gregario","migratorio":false},"tamano_promedio_cm":11,"peso_promedio_g":250,"reproduccion":"oviparo","epoca_reproductiva":"Deposita cápsulas ovígeras adheridas a la roca; la larva velígera permanece meses en el plancton."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Ficha de Concholepas concholepas","IFOP — Estado de las pesquerías bentónicas"]'
    ),
    (
        'Mytilus chilensis', 'Chorito', 'Hupé, 1854',
        'Bivalvo mitílido de valvas alargadas y de contorno triangular, color azul oscuro a negro con el interior nacarado, que se fija al sustrato mediante el biso, un haz de filamentos proteicos secretados por el pie. Filtra grandes volúmenes de agua para capturar fitoplancton y materia orgánica particulada, y en esa función concentra también toxinas de marea roja cuando hay floraciones nocivas, razón por la cual el monitoreo sanitario es permanente en el archipiélago. Es la base de la mitilicultura chilena: la captación natural de semilla en los estuarios de Chiloé abastece a toda la industria nacional, lo que convierte a esta especie en el principal producto de exportación acuícola de la región después del salmón.',
        'Bancos naturales en el intermareal y submareal somero de estuarios y bahías protegidas, sobre sustrato duro.',
        'Abundante en los estuarios y bahías del mar interior; la captación de semilla del archipiélago sostiene la mitilicultura del país.',
        false, 'No Evaluada', 'animalia', 'Mytilus',
        '{"clase":"Bivalvia","alimentacion":"filtrador","dieta_detalle":"Fitoplancton, detrito orgánico y bacterias filtradas del agua mediante las branquias; puede filtrar varios litros por hora.","comportamiento":{"actividad":"catemeral","social":"gregario","migratorio":false},"tamano_promedio_cm":7,"peso_promedio_g":25,"reproduccion":"oviparo","epoca_reproductiva":"Desove masivo en primavera y verano; la larva permanece semanas en el plancton antes de fijarse."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Mitilicultura","IFOP — Programa de sanidad de moluscos bivalvos"]'
    ),
    (
        'Aulacomya atra', 'Cholga', '(Molina, 1782)',
        'Mitílido de mayor tamaño que el chorito, con valvas de color pardo violáceo, superficie recorrida por costillas radiales bien marcadas y un interior nacarado con borde oscuro. Forma bancos densos en el submareal, adheridos entre sí y a la roca por el biso, y esos agregados generan un hábitat tridimensional que alberga una fauna asociada abundante de crustáceos, poliquetos y equinodermos. Se extrae mediante buceo semiautónomo y tiene un lugar tradicional en la cocina chilota: es uno de los mariscos característicos del curanto, junto con el chorito y la almeja, y también se consume seca y ensartada, en la forma conocida localmente como cholgas ahumadas.',
        'Submareal rocoso hasta varias decenas de metros, en bancos densos sobre roca y conchilla, en aguas con buena circulación.',
        'Presente en bancos submareales del mar interior y de la costa expuesta, extraída por buceo artesanal.',
        false, 'No Evaluada', 'animalia', 'Aulacomya',
        '{"clase":"Bivalvia","alimentacion":"filtrador","dieta_detalle":"Fitoplancton y materia orgánica particulada filtrada del agua circundante.","comportamiento":{"actividad":"catemeral","social":"gregario","migratorio":false},"tamano_promedio_cm":12,"peso_promedio_g":80,"reproduccion":"oviparo","epoca_reproductiva":"Desove estacional con fase larval planctónica de varias semanas."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Recursos bentónicos","IFOP — Estado de las pesquerías bentónicas"]'
    ),
    (
        'Choromytilus chorus', 'Choro zapato', '(Molina, 1782)',
        'El mayor de los mitílidos chilenos: puede superar los veinte centímetros de longitud, con valvas alargadas, de color negro azulado en el exterior y un interior nacarado de tonos violáceos. Vive semienterrado en fondos blandos de arena y fango del submareal somero, anclado por un biso robusto, a diferencia de sus parientes que se fijan sobre roca. Ese hábito lo hace vulnerable a la extracción con rastras y al deterioro del fondo. Fue históricamente un recurso abundante y muy valorado en la cocina del sur, y la sobreexplotación de sus bancos naturales redujo de manera drástica su disponibilidad, al punto de que hoy buena parte de la oferta proviene de bancos manejados y no de extracción libre.',
        'Fondos blandos de arena y fango del submareal somero en bahías y estuarios protegidos.',
        'Presente en bancos de fondo blando del mar interior; sus poblaciones naturales están muy reducidas respecto de las históricas.',
        false, 'No Evaluada', 'animalia', 'Choromytilus',
        '{"clase":"Bivalvia","alimentacion":"filtrador","dieta_detalle":"Fitoplancton y detrito orgánico filtrado del agua; como todo bivalvo filtrador puede acumular toxinas marinas.","comportamiento":{"actividad":"catemeral","social":"gregario","migratorio":false},"tamano_promedio_cm":18,"peso_promedio_g":250,"reproduccion":"oviparo","epoca_reproductiva":"Desove en primavera y verano, con larva planctónica antes del asentamiento."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Recursos bentónicos","IFOP — Estado de las pesquerías bentónicas"]'
    ),
    (
        'Ameghinomya antiqua', 'Almeja', '(King, 1832)',
        'Bivalvo de concha sólida, gruesa y de contorno redondeado, blanquecina o parda, con líneas concéntricas de crecimiento bien marcadas que permiten estimar la edad del individuo. Vive enterrada a pocos centímetros en fondos de arena y arena fangosa, manteniendo contacto con el agua mediante dos sifones cortos: uno inhalante, por el que ingresa el agua que filtra, y otro exhalante. Se desplaza y se entierra con un pie muscular en forma de hacha. Es uno de los mariscos tradicionales del archipiélago, presente en el curanto y en la preparación de mariscos secos, y se extrae por buceo o a mano en las playas durante las mareas bajas de sicigia.',
        'Fondos de arena y arena fangosa del intermareal bajo y submareal somero, en bahías y estuarios protegidos.',
        'Frecuente en playas y bancos arenosos del mar interior; recurso habitual de la extracción de orilla en mareas bajas.',
        false, 'No Evaluada', 'animalia', 'Ameghinomya',
        '{"clase":"Bivalvia","alimentacion":"filtrador","dieta_detalle":"Fitoplancton y materia orgánica en suspensión, captada a través del sifón inhalante.","comportamiento":{"actividad":"catemeral","social":"gregario","migratorio":false},"tamano_promedio_cm":7,"peso_promedio_g":60,"reproduccion":"oviparo","epoca_reproductiva":"Desove estacional; el asentamiento larval ocurre sobre fondos blandos tras semanas en el plancton."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Recursos bentónicos","IFOP — Estado de las pesquerías bentónicas"]'
    ),
    (
        'Loxechinus albus', 'Erizo rojo', '(Molina, 1782)',
        'Equinodermo de simetría radial pentámera, con un caparazón calcáreo rígido cubierto de espinas cortas y móviles de color verdoso a rojizo, y de pies ambulacrales que emergen entre ellas y le permiten adherirse y desplazarse. La boca, en la cara inferior, aloja la linterna de Aristóteles, un aparato masticador de cinco piezas con el que raspa y corta algas. Lo que se consume son las gónadas, de color anaranjado. Pastorea bosques de huiro y su abundancia influye directamente en la extensión de esos bosques submarinos. Chile es el principal productor mundial de esta pesquería, y su manejo —vedas, tallas mínimas y áreas de manejo— es materia de discusión constante en las caletas del sur.',
        'Submareal rocoso con bosques de huiro, desde pocos metros hasta más de cien de profundidad.',
        'Extraído por buceo en la costa expuesta y en canales del archipiélago; recurso económico central de varias caletas.',
        false, 'No Evaluada', 'animalia', 'Loxechinus',
        '{"clase":"Echinoidea","alimentacion":"herbivoro","dieta_detalle":"Macroalgas pardas y rojas, principalmente huiro, raspadas con la linterna de Aristóteles; también consume detrito algal a la deriva.","comportamiento":{"actividad":"catemeral","social":"gregario","migratorio":false},"tamano_promedio_cm":9,"peso_promedio_g":180,"reproduccion":"oviparo","epoca_reproductiva":"Desove masivo sincronizado en primavera; larva pluteus planctónica de varias semanas."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Pesquería del erizo","IFOP — Estado de las pesquerías bentónicas"]'
    ),
    (
        'Ceroglossus chilensis', 'Peorro', '(Eschscholtz, 1829)',
        'Escarabajo carábido de tamaño mediano y élitros de una iridiscencia notable, que según la población y el ángulo de la luz va del verde metálico al azul, el cobre o el violeta. Es áptero: los élitros están fusionados y ha perdido la capacidad de volar, lo que restringe su dispersión y explica la marcada diferenciación geográfica entre poblaciones aisladas, un rasgo que lo ha convertido en modelo de estudios de biogeografía del bosque templado. Es un depredador nocturno que recorre el suelo y la madera en descomposición cazando invertebrados de cuerpo blando. Su nombre común alude al líquido de olor penetrante que expulsa como defensa cuando se lo manipula.',
        'Suelo, hojarasca y madera en descomposición del bosque templado húmedo; también bajo cortezas y troncos caídos.',
        'Presente en bosques nativos de la isla; las poblaciones insulares muestran coloraciones propias.',
        false, 'No Evaluada', 'animalia', 'Ceroglossus',
        '{"clase":"Insecta","alimentacion":"carnivoro","dieta_detalle":"Invertebrados de cuerpo blando del mantillo: babosas, lombrices, larvas de insecto y caracoles pequeños.","comportamiento":{"actividad":"nocturno","social":"solitario","migratorio":false},"tamano_promedio_cm":2.5,"peso_promedio_g":1,"reproduccion":"oviparo","epoca_reproductiva":"Postura en el suelo húmedo del bosque; las larvas también son depredadoras y viven en la hojarasca."}',
        '["Museo Nacional de Historia Natural de Chile — Colección entomológica","Revista Chilena de Historia Natural — Biogeografía de Ceroglossus"]'
    ),
    (
        'Chiasognathus grantii', 'Ciervo volante de Darwin', 'Stephens, 1831',
        'Coleóptero lucánido de aspecto espectacular: el macho luce mandíbulas alargadas, curvas y dentadas que pueden alcanzar la mitad de la longitud de su cuerpo, y un tegumento pardo con reflejos metálicos verdosos y dorados. Esas mandíbulas no sirven para alimentarse sino para los combates rituales entre machos, que se enfrentan e intentan levantar y desalojar al rival de la rama en disputa; la hembra las tiene mucho más cortas. Darwin quedó impresionado por la especie durante el viaje del Beagle y la mencionó al discutir la selección sexual. Los adultos se alimentan de savia que fluye de heridas en los árboles, mientras que las larvas se desarrollan durante años dentro de madera muerta en descomposición, de modo que la especie depende de bosques con troncos caídos y madera muerta en pie.',
        'Bosques de Nothofagus y bosque templado maduro con abundante madera muerta donde se desarrollan las larvas.',
        'Presente en bosques bien conservados de la isla; los adultos se ven en verano, atraídos por la savia y las luces.',
        false, 'No Evaluada', 'animalia', 'Chiasognathus',
        '{"clase":"Insecta","alimentacion":"frugivoro","dieta_detalle":"Los adultos liban savia que rezuma de heridas de árboles y jugos de frutos fermentados; las larvas consumen madera muerta en descomposición durante varios años.","comportamiento":{"actividad":"diurno","social":"solitario","migratorio":false},"tamano_promedio_cm":8,"peso_promedio_g":3,"reproduccion":"oviparo","epoca_reproductiva":"Los adultos emergen en verano; la hembra deposita los huevos en madera muerta y el ciclo larval dura varios años."}',
        '["Museo Nacional de Historia Natural de Chile — Colección entomológica","Darwin, C. — El origen del hombre y la selección en relación al sexo"]'
    )
) AS v(
    nombre_cientifico, nombre_comun, autor_cientifico, descripcion, habitat,
    distribucion_chiloe, endemica, estado_conservacion, reino, genero,
    atributos, fuentes
)
JOIN generos g ON g.nombre = v.genero
JOIN familias f ON f.id = g.familia_id AND f.reino = v.reino::reino_enum
ON CONFLICT (nombre_cientifico) DO NOTHING;

-- =============================================================================
-- PLANTAE
-- =============================================================================

INSERT INTO familias (nombre, reino, descripcion) VALUES
    ('Nothofagaceae',        'plantae', 'Nothofagus: el género dominante de los bosques templados del hemisferio sur.'),
    ('Podocarpaceae',        'plantae', 'Coníferas australes de hojas planas y semilla con receptáculo carnoso.'),
    ('Cunoniaceae',          'plantae', 'Árboles y arbustos australes de hojas opuestas y flores pequeñas agrupadas.'),
    ('Atherospermataceae',   'plantae', 'Árboles de corteza y hojas fuertemente aromáticas.'),
    ('Aextoxicaceae',        'plantae', 'Familia monotípica endémica del bosque templado sudamericano.'),
    ('Proteaceae',           'plantae', 'Proteáceas: raíces proteoides y flores agrupadas en inflorescencias vistosas.'),
    ('Berberidaceae',        'plantae', 'Arbustos espinosos de madera amarilla y frutos en baya.'),
    ('Philesiaceae',         'plantae', 'Monocotiledóneas leñosas endémicas del bosque templado chileno.'),
    ('Poaceae',              'plantae', 'Gramíneas, incluidos los bambúes leñosos del sotobosque austral.'),
    ('Onagraceae',           'plantae', 'Hierbas y arbustos de flores tetrámeras, muchas ornitófilas.'),
    ('Ericaceae',            'plantae', 'Ericáceas: arbustos de suelos ácidos con micorrizas especializadas.'),
    ('Sphagnaceae',          'plantae', 'Musgos turbosos, constructores de turberas.'),
    ('Dicksoniaceae',        'plantae', 'Helechos arborescentes y afines del hemisferio sur.'),
    ('Blechnaceae',          'plantae', 'Helechos de frondas dimórficas, con hojas fértiles distintas de las estériles.'),
    ('Columelliaceae',       'plantae', 'Arbustos andino-patagónicos de hojas coriáceas.'),
    ('Alstroemeriaceae',     'plantae', 'Monocotiledóneas sudamericanas de hojas resupinadas.')
ON CONFLICT (reino, nombre) DO NOTHING;

INSERT INTO generos (nombre, familia_id, descripcion)
SELECT v.nombre, f.id, v.descripcion
FROM (VALUES
    ('Nothofagus',      'plantae', 'Nothofagaceae',      'Coigües, robles, ñirres y lengas.'),
    ('Pilgerodendron',  'plantae', 'Cupressaceae',       'Género monotípico del ciprés de las Guaitecas.'),
    ('Podocarpus',      'plantae', 'Podocarpaceae',      'Mañíos de hoja punzante.'),
    ('Saxegothaea',     'plantae', 'Podocarpaceae',      'Género monotípico de mañío hembra, con cono carnoso.'),
    ('Amomyrtus',       'plantae', 'Myrtaceae',          'Mirtáceas de hoja aromática del bosque valdiviano.'),
    ('Ugni',            'plantae', 'Myrtaceae',          'Murtas, arbustos de frutos comestibles muy aromáticos.'),
    ('Tepualia',        'plantae', 'Myrtaceae',          'Género monotípico del tepú, formador de tepuales.'),
    ('Eucryphia',       'plantae', 'Cunoniaceae',        'Ulmos, árboles de floración blanca masiva.'),
    ('Weinmannia',      'plantae', 'Cunoniaceae',        'Tineos, de hojas compuestas con raquis alado.'),
    ('Laureliopsis',    'plantae', 'Atherospermataceae', 'Género monotípico de la tepa.'),
    ('Laurelia',        'plantae', 'Atherospermataceae', 'Laureles chilenos de madera aromática.'),
    ('Aextoxicon',      'plantae', 'Aextoxicaceae',      'Género monotípico del olivillo.'),
    ('Embothrium',      'plantae', 'Proteaceae',         'Notros, de floración roja ornitófila.'),
    ('Gevuina',         'plantae', 'Proteaceae',         'Avellanos chilenos, de fruto comestible.'),
    ('Lomatia',         'plantae', 'Proteaceae',         'Fuinques y radales.'),
    ('Berberis',        'plantae', 'Berberidaceae',      'Michays y calafates, arbustos espinosos.'),
    ('Lapageria',       'plantae', 'Philesiaceae',       'Género monotípico del copihue, flor nacional de Chile.'),
    ('Philesia',        'plantae', 'Philesiaceae',       'Género monotípico del coicopihue.'),
    ('Chusquea',        'plantae', 'Poaceae',            'Quilas y colihues, bambúes leñosos del sotobosque.'),
    ('Fuchsia',         'plantae', 'Onagraceae',         'Chilcos, arbustos de flores colgantes.'),
    ('Gaultheria',      'plantae', 'Ericaceae',          'Chauras, arbustos de frutos carnosos coloridos.'),
    ('Sphagnum',        'plantae', 'Sphagnaceae',        'Musgos pompón, formadores de turba.'),
    ('Lophosoria',      'plantae', 'Dicksoniaceae',      'Palmillas, helechos de fronda grande y glauca.'),
    ('Blechnum',        'plantae', 'Blechnaceae',        'Quilquiles y costillas de vaca.'),
    ('Desfontainia',    'plantae', 'Columelliaceae',     'Taiques, arbustos de hoja aserrada y flor tubular roja.'),
    ('Luzuriaga',       'plantae', 'Alstroemeriaceae',   'Quilinejas, trepadoras y rastreras del sotobosque.')
) AS v(nombre, reino, familia, descripcion)
JOIN familias f ON f.nombre = v.familia AND f.reino = v.reino::reino_enum
ON CONFLICT (familia_id, nombre) DO NOTHING;

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
        'Nothofagus dombeyi', 'Coigüe', '(Mirb.) Oerst.',
        'Árbol siempreverde de gran porte y una de las especies estructurales del bosque valdiviano: supera con frecuencia los cuarenta metros y forma el dosel superior bajo el cual se organiza el resto de la comunidad. El tronco es recto y cilíndrico, con corteza gris que se agrieta en placas con la edad, y la copa se dispone en pisos horizontales bien diferenciados que dan al árbol una silueta reconocible desde lejos. Las hojas son pequeñas, coriáceas y de borde aserrado, dispuestas en ramillas aplanadas. Es una especie pionera de alta luminosidad: coloniza masivamente los claros abiertos por derrumbes o caídas de árboles, y forma rodales coetáneos que luego son reemplazados lentamente por especies tolerantes a la sombra.',
        'Bosque templado lluvioso en suelos con buen drenaje; coloniza claros y terrenos perturbados con abundante luz.',
        'Dominante en los bosques del interior y de la cordillera de la costa chilota, especialmente en laderas bien drenadas.',
        false, 'Preocupación Menor', 'plantae', 'Nothofagus',
        '{"tipo_planta":"arbol","altura_promedio_m":40,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[10,11,12],"fruto":{"descripcion":"Cúpula pequeña con tres nueces aladas que se dispersan con el viento.","comestible":"no_comestible"},"usos_tradicionales":["maderable"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Especies forestales del bosque templado"]'
    ),
    (
        'Nothofagus nitida', 'Coigüe de Chiloé', '(Phil.) Krasser',
        'Árbol siempreverde estrechamente asociado a los ambientes más húmedos y de peor drenaje del sur, donde reemplaza al coigüe común. Se distingue por sus hojas más pequeñas, brillantes y de borde finamente aserrado, y por sus ramillas oscuras. Tolera suelos anegados y ácidos que resultan hostiles para otras especies arbóreas, y por eso aparece en la transición entre el bosque siempreverde y las turberas, donde forma rodales de porte menor. Junto con el tepú y el ciprés de las Guaitecas caracteriza el paisaje forestal de la vertiente occidental de Chiloé, la más expuesta a la lluvia y al viento del Pacífico.',
        'Bosque siempreverde muy húmedo, suelos de drenaje pobre y bordes de turbera; tolera anegamiento estacional.',
        'Característico de la vertiente occidental de la Isla Grande y de los sectores de peor drenaje.',
        false, 'Preocupación Menor', 'plantae', 'Nothofagus',
        '{"tipo_planta":"arbol","altura_promedio_m":30,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[10,11,12],"fruto":{"descripcion":"Cúpula con nueces aladas dispersadas por el viento.","comestible":"no_comestible"},"usos_tradicionales":["maderable"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Especies forestales del bosque templado"]'
    ),
    (
        'Nothofagus betuloides', 'Coigüe de Magallanes', '(Mirb.) Oerst.',
        'La única especie arbórea siempreverde del género que alcanza el extremo austral del continente, donde forma el límite del bosque frente a la tundra magallánica. De hojas pequeñas, gruesas y de un verde muy oscuro, con margen crenado, presenta un porte que varía radicalmente con la exposición: erguido y de treinta metros en sitios protegidos, achaparrado y deformado por el viento en las costas expuestas, donde adopta la forma de bandera con todas las ramas orientadas a sotavento. Su madera es dura y pesada, y su capacidad de resistir suelos pobres, anegados y con salinidad aérea explica su presencia en los sectores más rigurosos del archipiélago.',
        'Bosque siempreverde austral, laderas expuestas al viento, suelos delgados y bordes de turbera.',
        'Presente en sectores expuestos y fríos del archipiélago, con formas achaparradas en la costa occidental.',
        false, 'Preocupación Menor', 'plantae', 'Nothofagus',
        '{"tipo_planta":"arbol","altura_promedio_m":25,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12],"fruto":{"descripcion":"Cúpula pequeña con nueces aladas.","comestible":"no_comestible"},"usos_tradicionales":["maderable"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Especies forestales del bosque templado"]'
    ),
    (
        'Nothofagus antarctica', 'Ñirre', '(G.Forst.) Oerst.',
        'La especie más plástica del género: puede presentarse como árbol de quince metros en sitios favorables o como arbusto retorcido de apenas un metro en turberas, orillas de mallines y sitios expuestos. Es caducifolio, y en otoño su follaje vira a tonos amarillos, anaranjados y rojos que tiñen el paisaje antes de la caída. Las hojas son pequeñas, de margen irregularmente crenado y algo onduladas, con un aroma característico al estrujarse. Tolera anegamiento, heladas fuertes, suelos pobres y viento, lo que lo convierte en colonizador de terrenos degradados y en el árbol que marca los límites del bosque tanto en altitud como en latitud.',
        'Turberas, mallines, bordes de bosque, laderas expuestas y suelos anegados o degradados.',
        'Presente en turberas y sectores de suelo pobre de la isla, con frecuencia en forma arbustiva.',
        false, 'Preocupación Menor', 'plantae', 'Nothofagus',
        '{"tipo_planta":"arbol","altura_promedio_m":10,"tipo_hoja":{"ciclo":"caduca","morfologia":"simple"},"floracion_meses":[10,11],"fruto":{"descripcion":"Cúpula con nueces aladas; la hojarasca caduca aporta materia orgánica al suelo.","comestible":"no_comestible"},"usos_tradicionales":["maderable","forraje"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Especies forestales del bosque templado"]'
    ),
    (
        'Pilgerodendron uviferum', 'Ciprés de las Guaitecas', '(D.Don) Florin',
        'La conífera de distribución más austral del mundo y uno de los árboles más característicos de los ambientes anegados del sur de Chile. De copa columnar y estrecha, follaje escamoso dispuesto en cuatro hileras y corteza fibrosa que se desprende en tiras, crece con extrema lentitud: individuos de pocos centímetros de diámetro pueden tener más de un siglo. Su madera es liviana, aromática y excepcionalmente resistente a la pudrición, cualidad que la hizo el material predilecto para tejuelas, postes, embarcaciones y las iglesias de madera de Chiloé. Esa misma virtud provocó una explotación intensa y prolongada que, sumada a los incendios de habilitación de terrenos, redujo drásticamente sus poblaciones; hoy está protegido como monumento natural.',
        'Turberas, suelos permanentemente anegados y de drenaje nulo, bordes de pantano y sectores costeros muy húmedos.',
        'Presente en turberas y suelos anegados de la Isla Grande, especialmente en la vertiente occidental; sus rodales están muy reducidos respecto de los históricos.',
        false, 'Vulnerable', 'plantae', 'Pilgerodendron',
        '{"tipo_planta":"arbol","altura_promedio_m":20,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12],"fruto":{"descripcion":"Cono pequeño y leñoso con escasas semillas aladas; la regeneración natural es muy lenta.","comestible":"no_comestible"},"usos_tradicionales":["maderable","ceremonial"],"tipo_raiz":"fibrosa","polinizacion":"anemofila"}',
        '["IUCN Red List — Pilgerodendron uviferum","CONAF — Monumentos naturales de Chile"]'
    ),
    (
        'Podocarpus nubigenus', 'Mañío macho', 'Lindl.',
        'Conífera siempreverde de porte mediano a grande, con hojas lineales, rígidas y punzantes, de un verde oscuro por el haz y con dos bandas blanquecinas de estomas por el envés que le dan un aspecto glauco al observarlas desde abajo. A diferencia de las coníferas del hemisferio norte, no produce piñas leñosas: la semilla queda expuesta sobre un receptáculo carnoso y coloreado que atrae a las aves, responsables de su dispersión. Crece en el interior del bosque siempreverde, tolera la sombra durante sus primeros años y alcanza el dosel lentamente. Su madera, de grano fino y sin resinas, fue apreciada en carpintería e interiores.',
        'Interior del bosque siempreverde templado, en suelos húmedos y bien drenados, generalmente en sitios sombríos.',
        'Presente en bosques siempreverdes del interior de la isla, en densidades bajas y disperso entre el dosel.',
        false, 'Preocupación Menor', 'plantae', 'Podocarpus',
        '{"tipo_planta":"arbol","altura_promedio_m":25,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12],"fruto":{"descripcion":"Semilla expuesta sobre un receptáculo carnoso y coloreado, dispersada por aves.","comestible":"no_comestible"},"usos_tradicionales":["maderable"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Especies forestales del bosque templado"]'
    ),
    (
        'Saxegothaea conspicua', 'Mañío hembra', 'Lindl.',
        'Conífera endémica del bosque templado sudamericano y única especie de su género, considerada un fósil viviente por su combinación de caracteres intermedios entre familias de coníferas. Sus hojas son lineales y dispuestas de forma dística, más blandas que las del mañío macho, y su rasgo distintivo es el cono femenino: una estructura globosa, carnosa y de aspecto azulado cubierta de escamas agudas, muy distinta de cualquier otra conífera austral. Es una especie tolerante a la sombra que se regenera bajo el dosel cerrado y puede permanecer décadas en el sotobosque esperando la apertura de un claro para crecer.',
        'Interior del bosque siempreverde húmedo y umbrío, en quebradas y laderas protegidas.',
        'Presente en bosques siempreverdes maduros de la isla, generalmente en el estrato intermedio.',
        false, 'Preocupación Menor', 'plantae', 'Saxegothaea',
        '{"tipo_planta":"arbol","altura_promedio_m":20,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12],"fruto":{"descripcion":"Cono globoso, carnoso y azulado con escamas agudas, muy característico del género.","comestible":"no_comestible"},"usos_tradicionales":["maderable"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","IUCN Red List — Saxegothaea conspicua"]'
    ),
    (
        'Amomyrtus luma', 'Luma blanca', '(Molina) D.Legrand & Kausel',
        'Mirtácea siempreverde de hojas opuestas, coriáceas y fuertemente aromáticas al estrujarse, con un olor resinoso característico que permite identificarla sin verla. Produce una floración blanca abundante que atrae a una gran diversidad de insectos, seguida de bayas oscuras que son alimento clave para zorzales, tordos y otras aves frugívoras del bosque. Su madera es de las más duras y pesadas de la flora chilena: se hunde en agua y se usó históricamente para mangos de herramienta, ejes y piezas sometidas a desgaste. Forma parte del estrato intermedio del bosque valdiviano, con frecuencia junto al tepú y el arrayán en sitios húmedos.',
        'Bosque siempreverde húmedo, quebradas, riberas y sectores de suelo permanentemente fresco.',
        'Frecuente en bosques húmedos y quebradas de toda la isla.',
        false, 'Preocupación Menor', 'plantae', 'Amomyrtus',
        '{"tipo_planta":"arbol","altura_promedio_m":15,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12,1],"fruto":{"descripcion":"Baya globosa negro-violácea, dulce, alimento importante para aves frugívoras del bosque.","comestible":"comestible"},"usos_tradicionales":["maderable","medicinal","alimentario"],"tipo_raiz":"axonomorfa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Ugni molinae', 'Murta', 'Turcz.',
        'Arbusto siempreverde de un a dos metros, con hojas pequeñas, opuestas, coriáceas y de borde revoluto, y flores solitarias, colgantes y acampanadas, blancas o rosadas. Su fruto es una baya rojiza de aroma intenso y sabor dulce y perfumado, uno de los productos silvestres más apreciados del sur de Chile: se consume fresca, en mermeladas, en licores y en el murtado, la preparación tradicional con aguardiente. La recolección de murta en verano es una actividad familiar arraigada en el campo chilote. Crece en bordes de bosque, claros y sectores abiertos, y responde bien a la perturbación moderada, por lo que abunda en cercos y renovales.',
        'Bordes de bosque, claros, matorrales y renovales con buena luz; suelos ácidos y bien drenados.',
        'Abundante en bordes de bosque, cercos y matorrales de toda la isla; su recolección estival es tradicional.',
        false, 'Preocupación Menor', 'plantae', 'Ugni',
        '{"tipo_planta":"arbusto","altura_promedio_m":1.5,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12],"fruto":{"descripcion":"Baya rojiza de aroma intenso y sabor dulce, madura entre febrero y abril; base del murtado y de mermeladas tradicionales.","comestible":"comestible"},"usos_tradicionales":["alimentario","medicinal","ornamental"],"tipo_raiz":"fibrosa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","INIA — Frutales nativos del sur de Chile"]'
    ),
    (
        'Tepualia stipularis', 'Tepú', '(Hook. & Arn.) Griseb.',
        'Arbusto o arbolito siempreverde de troncos múltiples, retorcidos y entrelazados, con corteza rojiza que se desprende en tiras. Forma los tepuales: formaciones casi impenetrables sobre suelos permanentemente anegados donde las ramas y los troncos caídos se superponen a un metro o más sobre el nivel del agua, de modo que se camina literalmente sobre la vegetación y no sobre el suelo. Es una de las especies más características del paisaje de la vertiente occidental de Chiloé. Su madera es dura y de combustión lenta, muy valorada como leña y para postes, y su capacidad de rebrotar desde la base le permite recuperarse de la corta.',
        'Suelos anegados y turbosos, bordes de pantano y depresiones sin drenaje del bosque siempreverde.',
        'Forma tepuales extensos en la vertiente occidental y en los sectores de peor drenaje de la Isla Grande.',
        false, 'Preocupación Menor', 'plantae', 'Tepualia',
        '{"tipo_planta":"arbusto","altura_promedio_m":6,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[12,1,2],"fruto":{"descripcion":"Cápsula pequeña y leñosa con semillas diminutas.","comestible":"no_comestible"},"usos_tradicionales":["maderable"],"tipo_raiz":"fibrosa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Formaciones vegetacionales de Chiloé"]'
    ),
    (
        'Eucryphia cordifolia', 'Ulmo', 'Cav.',
        'Árbol siempreverde de gran porte y uno de los más vistosos del bosque valdiviano durante el verano, cuando se cubre por completo de flores blancas de cuatro pétalos y numerosos estambres, hasta el punto de que la copa parece nevada. Esa floración masiva y sincronizada sostiene una actividad de insectos enorme y es la base de la miel de ulmo, un producto monofloral de aroma y sabor distintivos y con reconocimiento internacional, uno de los principales productos apícolas del sur de Chile. Las hojas son opuestas, coriáceas y de base acorazonada. Su corteza fue usada para curtir cueros y su madera, dura y de buen comportamiento, en construcción y carpintería.',
        'Bosque siempreverde templado en suelos profundos y húmedos con buen drenaje; frecuente en quebradas y laderas.',
        'Presente en bosques bien conservados de la isla; su floración de verano es visible a distancia por el blanco de las copas.',
        false, 'Preocupación Menor', 'plantae', 'Eucryphia',
        '{"tipo_planta":"arbol","altura_promedio_m":30,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[1,2,3],"fruto":{"descripcion":"Cápsula leñosa que se abre en valvas liberando semillas aladas.","comestible":"no_comestible"},"usos_tradicionales":["maderable","medicinal","tintoreo"],"tipo_raiz":"axonomorfa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","INIA — Mieles monoflorales del sur de Chile"]'
    ),
    (
        'Weinmannia trichosperma', 'Tineo', 'Cav.',
        'Árbol siempreverde de hojas compuestas e imparipinnadas con el raquis alado y foliolos aserrados, un carácter que lo distingue de inmediato entre las especies del bosque templado. Florece en racimos blancos densos y abundantes que atraen a numerosos insectos, y su fruto es una cápsula pequeña que libera semillas diminutas dispersadas por el viento, lo que le permite colonizar rápidamente terrenos abiertos. Es una especie pionera de crecimiento veloz en claros y bordes, y forma rodales puros en sitios perturbados. Su corteza es rica en taninos y se utilizó intensamente en curtiembres, actividad que motivó su explotación durante buena parte del siglo XX.',
        'Bordes de bosque, claros, renovales y sitios perturbados con buena luz; suelos húmedos y ácidos.',
        'Común en renovales y bordes de bosque de la isla, indicador de recuperación tras la perturbación.',
        false, 'Preocupación Menor', 'plantae', 'Weinmannia',
        '{"tipo_planta":"arbol","altura_promedio_m":25,"tipo_hoja":{"ciclo":"perenne","morfologia":"compuesta"},"floracion_meses":[11,12,1],"fruto":{"descripcion":"Cápsula diminuta con semillas muy pequeñas dispersadas por el viento.","comestible":"no_comestible"},"usos_tradicionales":["maderable","tintoreo"],"tipo_raiz":"axonomorfa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Especies forestales del bosque templado"]'
    ),
    (
        'Laureliopsis philippiana', 'Tepa', '(Looser) Schodde',
        'Árbol siempreverde de tronco recto y copa densa, con hojas opuestas, coriáceas, de borde aserrado y un aroma penetrante y algo alcanforado al estrujarse, rasgo compartido por toda la familia. Prefiere los sitios más húmedos y sombríos del bosque siempreverde, en quebradas y fondos de valle, y tolera bien la sombra durante su desarrollo juvenil. Su madera es liviana, blanca y fácil de trabajar, muy usada en revestimientos, tejuelas y muebles, aunque poco durable a la intemperie. En la medicina tradicional del sur sus hojas se emplearon en infusiones y sahumerios por sus propiedades aromáticas.',
        'Bosque siempreverde húmedo y umbrío, quebradas, fondos de valle y riberas.',
        'Frecuente en las quebradas y sectores húmedos y sombríos del bosque de la isla.',
        false, 'Preocupación Menor', 'plantae', 'Laureliopsis',
        '{"tipo_planta":"arbol","altura_promedio_m":30,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[10,11,12],"fruto":{"descripcion":"Fruto seco con semillas provistas de pelos plumosos que facilitan la dispersión por el viento.","comestible":"no_comestible"},"usos_tradicionales":["maderable","medicinal"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Laurelia sempervirens', 'Laurel chileno', '(Ruiz & Pav.) Tul.',
        'Árbol siempreverde de gran altura, con hojas opuestas, brillantes, de borde ondulado-aserrado y un aroma intenso y agradable que se libera al romperlas, muy distinto del laurel europeo aunque igualmente usado como condimento en la cocina tradicional del sur. El tronco es recto y de corteza lisa y grisácea en los ejemplares jóvenes. Crece en suelos profundos y húmedos, con frecuencia en fondos de valle y riberas donde la disponibilidad de agua es constante. Su madera, liviana y de buena trabajabilidad, se utilizó en construcción interior, y sus hojas tienen uso medicinal tradicional en infusiones para afecciones respiratorias.',
        'Suelos profundos y húmedos de fondos de valle, riberas y quebradas del bosque templado.',
        'Presente en sectores húmedos y riberas de la isla, aunque menos abundante que la tepa.',
        false, 'Preocupación Menor', 'plantae', 'Laurelia',
        '{"tipo_planta":"arbol","altura_promedio_m":30,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[9,10,11],"fruto":{"descripcion":"Fruto seco con semillas plumosas dispersadas por el viento.","comestible":"no_comestible"},"usos_tradicionales":["maderable","medicinal","alimentario"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Aextoxicon punctatum', 'Olivillo', 'Ruiz & Pav.',
        'Árbol siempreverde y única especie de su familia, un linaje aislado sin parientes cercanos vivos que representa uno de los elementos más antiguos de la flora del bosque templado sudamericano. Sus hojas son opuestas, coriáceas y cubiertas en el envés por escamas diminutas que le dan un aspecto puntuado y algo plateado, carácter del que deriva el epíteto específico. Es dioico: hay pies masculinos y femeninos separados, y solo los segundos producen los frutos ovoides y oscuros, parecidos a aceitunas pequeñas, que alimentan a las aves. Forma bosques densos y casi puros en la franja costera expuesta a la neblina, los llamados olivillos costeros, formación de altísimo valor de conservación.',
        'Franja costera con influencia de neblina, laderas expuestas al océano y quebradas húmedas cercanas al mar.',
        'Presente en la costa occidental de la isla, donde forma rodales densos asociados a la neblina marina.',
        false, 'Preocupación Menor', 'plantae', 'Aextoxicon',
        '{"tipo_planta":"arbol","altura_promedio_m":25,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[8,9,10],"fruto":{"descripcion":"Drupa ovoide oscura semejante a una aceituna pequeña, dispersada por aves; solo la producen los ejemplares femeninos.","comestible":"no_comestible"},"usos_tradicionales":["maderable"],"tipo_raiz":"axonomorfa","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","CONAF — Formaciones vegetacionales de Chile"]'
    ),
    (
        'Embothrium coccineum', 'Notro', 'J.R.Forst. & G.Forst.',
        'Árbol pequeño o arbolito de floración espectacular: entre la primavera y el comienzo del verano se cubre de inflorescencias de un rojo escarlata intenso formadas por flores tubulares que se abren enroscándose, un espectáculo que le ha dado el nombre alternativo de ciruelillo y que lo hace inconfundible a distancia en el borde del bosque. Esas flores son ornitófilas: su forma tubular, su color rojo y su néctar abundante están coevolucionados con el picaflor chico, su polinizador principal. Posee raíces proteoides, agrupaciones densas de raicillas que le permiten captar fósforo en los suelos pobres y ácidos del sur, y rebrota vigorosamente desde la raíz tras el fuego o la corta, comportándose como colonizador de terrenos abiertos.',
        'Bordes de bosque, claros, renovales y terrenos abiertos con suelos ácidos y pobres; muy tolerante a la perturbación.',
        'Común en bordes de camino, cercos y renovales de toda la isla; su floración roja marca la primavera chilota.',
        false, 'Preocupación Menor', 'plantae', 'Embothrium',
        '{"tipo_planta":"arbol","altura_promedio_m":10,"tipo_hoja":{"ciclo":"semicaduca","morfologia":"simple"},"floracion_meses":[10,11,12],"fruto":{"descripcion":"Folículo leñoso que se abre liberando semillas aladas dispersadas por el viento.","comestible":"no_comestible"},"usos_tradicionales":["ornamental","medicinal","maderable"],"tipo_raiz":"axonomorfa","polinizacion":"ornitofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Gevuina avellana', 'Avellano chileno', 'Molina',
        'Árbol siempreverde de hojas compuestas, brillantes y de foliolos aserrados, con una silueta abierta y elegante. Sus flores blanquecinas se disponen en racimos y dan lugar a frutos globosos que maduran cambiando de verde a rojo y finalmente a negro, de modo que un mismo racimo exhibe simultáneamente los tres colores, carácter muy llamativo en otoño. La semilla es comestible y de sabor semejante al de la avellana europea, aunque sin parentesco botánico: se consume tostada y es un producto recolectado tradicionalmente en el sur, además de fuente de un aceite de composición peculiar, rico en ácidos grasos monoinsaturados, con uso cosmético. Tolera la sombra y crece en el estrato intermedio del bosque.',
        'Interior y borde del bosque templado húmedo, en suelos profundos y bien drenados; tolera sombra parcial.',
        'Presente en bosques de la isla; sus frutos se recolectan en otoño en el campo chilote.',
        false, 'Preocupación Menor', 'plantae', 'Gevuina',
        '{"tipo_planta":"arbol","altura_promedio_m":15,"tipo_hoja":{"ciclo":"perenne","morfologia":"compuesta"},"floracion_meses":[1,2,3],"fruto":{"descripcion":"Nuez globosa que pasa de verde a rojo y luego a negro al madurar; la semilla se consume tostada y de ella se extrae un aceite apreciado.","comestible":"comestible"},"usos_tradicionales":["alimentario","maderable","ornamental"],"tipo_raiz":"axonomorfa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","INIA — Frutales nativos del sur de Chile"]'
    ),
    (
        'Lomatia ferruginea', 'Fuinque', '(Cav.) R.Br.',
        'Arbusto o arbolito siempreverde de aspecto muy ornamental, con hojas grandes, bipinnadas y de textura afelpada, cubiertas en el envés y en las ramillas jóvenes por una densa pubescencia de color ferrugíneo que le da nombre. Sus flores, agrupadas en racimos axilares, son de un amarillo verdoso teñido de rojo y presentan la estructura característica de las proteáceas, con el estilo que se libera con fuerza al abrirse la flor. Crece en el sotobosque húmedo y sombrío, con frecuencia en quebradas y a orillas de esteros, y como el resto de la familia posee raíces proteoides que le permiten prosperar en suelos pobres en fósforo.',
        'Sotobosque húmedo y sombrío del bosque siempreverde, quebradas y riberas de esteros.',
        'Frecuente en el sotobosque húmedo y en quebradas de los bosques de la isla.',
        false, 'Preocupación Menor', 'plantae', 'Lomatia',
        '{"tipo_planta":"arbusto","altura_promedio_m":5,"tipo_hoja":{"ciclo":"perenne","morfologia":"compuesta"},"floracion_meses":[12,1,2],"fruto":{"descripcion":"Folículo leñoso con semillas aladas dispersadas por el viento.","comestible":"no_comestible"},"usos_tradicionales":["ornamental","medicinal"],"tipo_raiz":"axonomorfa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Berberis darwinii', 'Michay', 'Hook.',
        'Arbusto siempreverde y espinoso descrito por Darwin durante el viaje del Beagle, de hojas pequeñas, coriáceas, brillantes y con tres a cinco dientes punzantes en el margen, semejantes a las de un acebo en miniatura. En primavera se cubre de racimos colgantes de flores de un naranja dorado intenso, y en verano produce bayas de color azul violáceo con pruina, comestibles, de sabor agridulce, usadas en mermeladas y licores. La madera y la raíz contienen berberina, un alcaloide amarillo que le da a la madera un color intenso y que sustentó su uso tradicional como tintóreo y medicinal. Es un arbusto pionero que forma matorrales densos en bordes y terrenos abiertos.',
        'Bordes de bosque, claros, cercos y matorrales; suelos variados, tolera exposición y perturbación.',
        'Muy común en cercos vivos y bordes de camino de toda la isla.',
        false, 'Preocupación Menor', 'plantae', 'Berberis',
        '{"tipo_planta":"arbusto","altura_promedio_m":2.5,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[9,10,11],"fruto":{"descripcion":"Baya azul violácea con pruina, agridulce, consumida fresca y en mermeladas; dispersada por aves.","comestible":"comestible"},"usos_tradicionales":["alimentario","medicinal","tintoreo","ornamental"],"tipo_raiz":"fibrosa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Berberis microphylla', 'Calafate', 'G.Forst.',
        'Arbusto espinoso de hojas pequeñas, coriáceas y agrupadas en fascículos, con espinas trífidas en la base de cada grupo. Sus flores solitarias o en pares son de un amarillo intenso, y el fruto es una baya de color azul oscuro casi negro, cubierta de pruina, de sabor dulce y astringente y con una concentración muy alta de antocianinas, lo que ha impulsado su interés comercial como fruto funcional. Es la especie asociada al dicho patagónico según el cual quien come calafate vuelve, y se consume fresca, en mermeladas, licores y postres. Tolera el viento, el frío y los suelos pobres, y coloniza terrenos degradados y bordes expuestos.',
        'Matorrales, bordes de bosque, terrenos abiertos y expuestos al viento, con suelos pobres y bien drenados.',
        'Presente en matorrales y sectores abiertos de la isla; sus frutos se recolectan a fines del verano.',
        false, 'Preocupación Menor', 'plantae', 'Berberis',
        '{"tipo_planta":"arbusto","altura_promedio_m":2,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[10,11],"fruto":{"descripcion":"Baya azul oscura con pruina, dulce y muy rica en antocianinas; base de mermeladas y licores tradicionales del sur.","comestible":"comestible"},"usos_tradicionales":["alimentario","medicinal","tintoreo"],"tipo_raiz":"fibrosa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","INIA — Frutales nativos del sur de Chile"]'
    ),
    (
        'Lapageria rosea', 'Copihue', 'Ruiz & Pav.',
        'Flor nacional de Chile y una de las plantas más emblemáticas del bosque templado: una trepadora leñosa de tallos volubles que asciende por los troncos hasta alcanzar la luz, con hojas coriáceas, ovadas y de nervadura paralela, propia de las monocotiledóneas. Sus flores son grandes, colgantes y acampanadas, formadas por seis tépalos carnosos de un rojo intenso —con variantes rosadas y blancas—, y producen néctar abundante en la base, lo que las liga estrechamente al picaflor chico como polinizador. El fruto es una baya verde amarillenta comestible, de pulpa dulce, que las aves dispersan. Su recolección indiscriminada llevó a que su corta esté regulada por ley.',
        'Interior y borde de bosque templado húmedo, trepando por troncos y arbustos en busca de luz.',
        'Presente en bosques húmedos de la isla; florece en verano y otoño en el interior del bosque.',
        false, 'Preocupación Menor', 'plantae', 'Lapageria',
        '{"tipo_planta":"trepadora","altura_promedio_m":10,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[2,3,4,5],"fruto":{"descripcion":"Baya verde amarillenta de pulpa dulce y numerosas semillas, dispersada por aves.","comestible":"comestible"},"usos_tradicionales":["ornamental","ceremonial","alimentario"],"tipo_raiz":"rizoma","polinizacion":"ornitofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Ministerio del Medio Ambiente de Chile — Flora nativa protegida"]'
    ),
    (
        'Philesia magellanica', 'Coicopihue', 'J.F.Gmel.',
        'Arbusto bajo y ramificado, pariente cercano del copihue pero de porte enteramente distinto: no trepa, sino que forma matas densas de menos de un metro y medio, con hojas pequeñas, rígidas, de borde revoluto y envés blanquecino. Sus flores son grandes en relación con la planta, colgantes, tubulares y de un rosado intenso, y aparecen en verano entre el follaje. Vive en los ambientes más húmedos y umbríos del bosque siempreverde, con frecuencia sobre troncos en descomposición, en bordes de turbera y en el sotobosque de los tepuales. Es una especie de crecimiento lento e indicadora de bosques bien conservados y de humedad constante.',
        'Sotobosque muy húmedo y umbrío, bordes de turbera, tepuales y troncos en descomposición.',
        'Presente en los sectores más húmedos y sombríos del bosque de la vertiente occidental de la isla.',
        false, 'Preocupación Menor', 'plantae', 'Philesia',
        '{"tipo_planta":"arbusto","altura_promedio_m":1.2,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[12,1,2,3],"fruto":{"descripcion":"Baya carnosa con numerosas semillas pequeñas.","comestible":"desconocido"},"usos_tradicionales":["ornamental"],"tipo_raiz":"rizoma","polinizacion":"ornitofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Chusquea quila', 'Quila', 'Kunth',
        'Bambú leñoso trepador de cañas macizas —a diferencia de la mayoría de los bambúes, que son huecos— que se apoya en la vegetación circundante y puede formar marañas impenetrables de varios metros de altura en claros y bordes. Su presencia estructura el sotobosque del bosque templado: donde la quila domina, la regeneración de las especies arbóreas queda suprimida hasta que la mata muere. Y muere de manera espectacular: florece de forma masiva y sincronizada cada varias décadas, fructifica y muere por completo, liberando una cantidad enorme de semilla que dispara explosiones poblacionales de roedores conocidas como ratadas, con implicancias sanitarias por el hantavirus. Sus cañas se usan tradicionalmente en cercos, cestería y construcción rural.',
        'Claros, bordes de bosque, renovales y sotobosque perturbado; forma marañas densas donde hay luz.',
        'Muy abundante en el sotobosque y los bordes de bosque de toda la isla; su manejo es un asunto habitual del campo chilote.',
        false, 'Preocupación Menor', 'plantae', 'Chusquea',
        '{"tipo_planta":"trepadora","altura_promedio_m":8,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12,1],"fruto":{"descripcion":"Cariopsis producida en floraciones masivas y sincronizadas cada varias décadas, tras las cuales la mata muere por completo.","comestible":"no_comestible"},"usos_tradicionales":["fibra","forraje"],"tipo_raiz":"rizoma","polinizacion":"anemofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Revista Chilena de Historia Natural — Floración masiva de Chusquea"]'
    ),
    (
        'Fuchsia magellanica', 'Chilco', 'Lam.',
        'Arbusto de ramas delgadas y arqueadas, hojas opuestas o verticiladas y flores colgantes inconfundibles: un tubo y cuatro sépalos de un rojo intenso que se abren hacia atrás, cuatro pétalos violáceos en el interior y estambres y estilo largamente exertos. Toda esa arquitectura es una adaptación a la polinización por picaflores, que liban en vuelo estacionario y quedan marcados con polen en la frente. Crece en suelos húmedos, en riberas de esteros, bordes de bosque y sitios con agua permanente, y florece durante un periodo largo, de primavera a otoño. Su fruto es una baya oscura comestible, y sus hojas tuvieron uso medicinal tradicional. Es el ancestro silvestre de buena parte de los fucsias de jardín cultivados en el mundo.',
        'Riberas de esteros, bordes de bosque húmedo, quebradas y sitios con humedad permanente.',
        'Común en riberas, zanjas y bordes húmedos de toda la isla, con floración prolongada.',
        false, 'Preocupación Menor', 'plantae', 'Fuchsia',
        '{"tipo_planta":"arbusto","altura_promedio_m":2.5,"tipo_hoja":{"ciclo":"caduca","morfologia":"simple"},"floracion_meses":[10,11,12,1,2,3,4],"fruto":{"descripcion":"Baya oblonga de color púrpura oscuro, dulce, comestible y dispersada por aves.","comestible":"comestible"},"usos_tradicionales":["ornamental","medicinal","alimentario"],"tipo_raiz":"fibrosa","polinizacion":"ornitofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Gaultheria mucronata', 'Chaura', '(L.f.) Hook. & Arn.',
        'Arbusto bajo y muy ramificado, con hojas pequeñas, rígidas, punzantes y de borde aserrado, y flores blancas o rosadas en forma de urna colgante, típicas de las ericáceas. Su fruto es una baya globosa y carnosa cuyo color varía notablemente entre individuos: blanco, rosado, rojo, púrpura o casi negro, de modo que un mismo matorral puede exhibir toda la gama. Esas bayas persisten mucho tiempo en la planta y son alimento de aves durante el otoño y el invierno. Crece en suelos ácidos y pobres, en bordes de turbera, matorrales y sectores expuestos, y depende de micorrizas ericoides para absorber nutrientes en esos sustratos difíciles.',
        'Bordes de turbera, matorrales, claros y suelos ácidos y pobres, tanto costeros como del interior.',
        'Muy común en matorrales, bordes de turbera y sectores abiertos de la isla.',
        false, 'Preocupación Menor', 'plantae', 'Gaultheria',
        '{"tipo_planta":"arbusto","altura_promedio_m":1,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[9,10,11],"fruto":{"descripcion":"Baya carnosa de color muy variable entre individuos —blanco, rosado, rojo o púrpura—, persistente y consumida por aves.","comestible":"comestible"},"usos_tradicionales":["ornamental","medicinal"],"tipo_raiz":"fibrosa","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Sphagnum magellanicum', 'Pompón', 'Brid.',
        'Musgo constructor de turberas y una de las especies con mayor capacidad de transformar su propio ambiente. Sus hojas contienen células hialinas muertas, grandes y porosas, que funcionan como esponjas y le permiten retener hasta veinte veces su peso seco en agua. Acidifica activamente el medio intercambiando cationes por hidrogeniones, lo que inhibe la descomposición y provoca que la materia orgánica se acumule en lugar de mineralizarse: así se forma la turba, capa a capa, durante milenios. Las turberas de pompón de Chiloé son reservorios de carbono y reguladores hídricos de primera importancia, y su extracción comercial para sustrato hortícola, en expansión desde los años dos mil, ha abierto un conflicto ambiental y regulatorio aún no resuelto.',
        'Turberas, pomponales, depresiones anegadas y suelos permanentemente saturados y ácidos.',
        'Forma extensos pomponales en la Isla Grande; su extracción para sustrato es una actividad económica relevante y controvertida.',
        false, 'No Evaluada', 'plantae', 'Sphagnum',
        '{"tipo_planta":"musgo","altura_promedio_m":0.3,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"usos_tradicionales":["medicinal","fibra"],"tipo_raiz":"adventicia"}',
        '["Ministerio del Medio Ambiente de Chile — Turberas de Chiloé","Revista Chilena de Historia Natural — Ecología de Sphagnum magellanicum"]'
    ),
    (
        'Lophosoria quadripinnata', 'Palmilla', '(J.F.Gmel.) C.Chr.',
        'Helecho de gran tamaño con frondas que superan los dos metros, tetrapinnadas y de un verde intenso por el haz, notablemente glaucas y blanquecinas por el envés debido a una cera que reduce la pérdida de agua. Emerge de un rizoma robusto y rastrero cubierto de pelos rojizos, y sus frondas jóvenes se despliegan enrolladas en el báculo característico de los helechos. Coloniza taludes húmedos, bordes de camino en el bosque y quebradas sombrías, donde forma poblaciones densas. Se reproduce por esporas depositadas en soros redondeados del envés, y su presencia indica humedad ambiental alta y constante durante todo el año.',
        'Taludes húmedos, quebradas sombrías, bordes de camino en el bosque y sotobosque con humedad constante.',
        'Abundante en taludes y quebradas húmedas del bosque de la isla.',
        false, 'Preocupación Menor', 'plantae', 'Lophosoria',
        '{"tipo_planta":"helecho","altura_promedio_m":2,"tipo_hoja":{"ciclo":"perenne","morfologia":"compuesta"},"usos_tradicionales":["ornamental"],"tipo_raiz":"rizoma"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Helechos de Chile"]'
    ),
    (
        'Blechnum chilense', 'Costilla de vaca', '(Kaulf.) Mett.',
        'Helecho robusto de frondas grandes, coriáceas y pinnadas, con las pinnas dispuestas de forma regular a ambos lados del raquis, disposición que recuerda a un costillar y le da su nombre común. Presenta dimorfismo foliar: las frondas fértiles, que aparecen en el centro de la mata, son más estrechas, erguidas y de color pardo, y llevan los esporangios en líneas continuas a lo largo del envés, mientras que las estériles son anchas y verdes. Crece a partir de un rizoma grueso y ascendente que con los años forma un tronco corto, y coloniza quebradas, riberas y taludes húmedos del interior del bosque, con frecuencia en poblaciones densas que cubren por completo el sotobosque.',
        'Quebradas, riberas de esteros, taludes húmedos y sotobosque sombrío del bosque templado.',
        'Muy frecuente en el sotobosque húmedo y en quebradas de toda la isla.',
        false, 'Preocupación Menor', 'plantae', 'Blechnum',
        '{"tipo_planta":"helecho","altura_promedio_m":1.5,"tipo_hoja":{"ciclo":"perenne","morfologia":"compuesta"},"usos_tradicionales":["ornamental"],"tipo_raiz":"rizoma"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Helechos de Chile"]'
    ),
    (
        'Desfontainia spinosa', 'Taique', 'Ruiz & Pav.',
        'Arbusto siempreverde de hojas opuestas, coriáceas, brillantes y con dientes espinosos en el margen, muy semejantes a las del acebo, lo que provoca confusiones frecuentes a primera vista. Sus flores son tubulares, largas, colgantes y de un rojo escarlata con el ápice amarillo, muy vistosas contra el follaje oscuro, y están adaptadas a la polinización por picaflores. Crece en el sotobosque húmedo y en bordes de turbera, tanto en la costa como en la montaña, y su crecimiento es lento. En la medicina tradicional mapuche y huilliche tuvo usos rituales y medicinales asociados a la machi, y se le atribuyen propiedades psicoactivas que justifican tratarla con cautela.',
        'Sotobosque húmedo del bosque siempreverde, bordes de turbera y matorrales de altura.',
        'Presente en el sotobosque húmedo y en bordes de turbera de la isla, con floración vistosa en verano.',
        false, 'Preocupación Menor', 'plantae', 'Desfontainia',
        '{"tipo_planta":"arbusto","altura_promedio_m":2,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[12,1,2,3],"fruto":{"descripcion":"Baya globosa blanquecina a amarillenta con numerosas semillas.","comestible":"no_comestible"},"usos_tradicionales":["ceremonial","medicinal","ornamental"],"tipo_raiz":"fibrosa","polinizacion":"ornitofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    ),
    (
        'Luzuriaga radicans', 'Quilineja', 'Ruiz & Pav.',
        'Planta de tallos delgados y flexibles que crece rastrera sobre el suelo, sobre troncos caídos y ascendiendo por los tallos de otras plantas, con hojas pequeñas, dísticas y resupinadas —torcidas en la base de modo que el envés queda hacia arriba—, carácter propio de su familia. Sus flores son blancas, estrelladas y colgantes, seguidas de bayas anaranjadas persistentes que destacan en la penumbra del sotobosque. Sus tallos, largos, resistentes y flexibles, se usaron tradicionalmente en Chiloé para amarras y cestería, y con ellos se confeccionaban las escobas y los cordeles de uso doméstico, motivo por el cual la quilineja está incorporada al oficio artesanal de la isla.',
        'Sotobosque húmedo y sombrío, sobre troncos caídos, musgos y bases de árboles del bosque siempreverde.',
        'Común en el interior del bosque húmedo de la isla; su uso en cestería es parte de la artesanía tradicional chilota.',
        false, 'Preocupación Menor', 'plantae', 'Luzuriaga',
        '{"tipo_planta":"epifita","altura_promedio_m":1,"tipo_hoja":{"ciclo":"perenne","morfologia":"simple"},"floracion_meses":[11,12,1],"fruto":{"descripcion":"Baya anaranjada persistente, consumida y dispersada por aves del sotobosque.","comestible":"desconocido"},"usos_tradicionales":["fibra","ornamental"],"tipo_raiz":"rizoma","polinizacion":"entomofila"}',
        '["Rodríguez, R. et al. — Flora de Chile","Hoffmann, A. — Flora silvestre de Chile"]'
    )
) AS v(
    nombre_cientifico, nombre_comun, autor_cientifico, descripcion, habitat,
    distribucion_chiloe, endemica, estado_conservacion, reino, genero,
    atributos, fuentes
)
JOIN generos g ON g.nombre = v.genero
JOIN familias f ON f.id = g.familia_id AND f.reino = v.reino::reino_enum
ON CONFLICT (nombre_cientifico) DO NOTHING;

-- =============================================================================
-- FUNGI
-- =============================================================================

INSERT INTO familias (nombre, reino, descripcion) VALUES
    ('Meripilaceae',     'fungi', 'Poliporales de carpóforo carnoso a coriáceo.'),
    ('Fistulinaceae',    'fungi', 'Hongos de himenio formado por tubos independientes entre sí.'),
    ('Amanitaceae',      'fungi', 'Agaricales con volva y anillo; incluye especies mortales.'),
    ('Lobariaceae',      'fungi', 'Líquenes foliáceos de gran tamaño, sensibles a la contaminación.'),
    ('Parmeliaceae',     'fungi', 'La familia más diversa de líquenes, incluye las barbas de viejo.')
ON CONFLICT (reino, nombre) DO NOTHING;

INSERT INTO generos (nombre, familia_id, descripcion)
SELECT v.nombre, f.id, v.descripcion
FROM (VALUES
    ('Grifola',           'fungi', 'Meripilaceae',  'Hongos de carpóforo ramificado en repisas.'),
    ('Fistulina',         'fungi', 'Fistulinaceae', 'Hongos carnosos parásitos de Nothofagus.'),
    ('Amanita',           'fungi', 'Amanitaceae',   'Agaricales con volva; incluye especies tóxicas y mortales.'),
    ('Pseudocyphellaria', 'fungi', 'Lobariaceae',   'Líquenes foliáceos con pseudocifelas en el envés.'),
    ('Usnea',             'fungi', 'Parmeliaceae',  'Líquenes fruticosos colgantes con cordón central elástico.')
) AS v(nombre, reino, familia, descripcion)
JOIN familias f ON f.nombre = v.familia AND f.reino = v.reino::reino_enum
ON CONFLICT (familia_id, nombre) DO NOTHING;

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
        'Cyttaria darwinii', 'Pan de indio', 'Berk.',
        'Ascomiceto parásito obligado de Nothofagus que forma agallas leñosas en las ramas del árbol hospedero; de esas agallas emergen en primavera cuerpos fructíferos globosos, carnosos y de color anaranjado a amarillento, cubiertos de depresiones circulares que les dan el aspecto de un panal. Darwin los describió durante el viaje del Beagle y consignó que los fueguinos los consumían crudos en cantidad, siendo uno de los pocos alimentos vegetales de su dieta. Son comestibles, de textura gelatinosa y sabor suave y algo dulce, y localmente se preparan en ensaladas. Su ciclo está enteramente ligado al del árbol: sin Nothofagus vivo no hay hongo.',
        'Ramas vivas de Nothofagus, sobre las agallas leñosas que el propio hongo induce en el hospedero.',
        'Presente en bosques de Nothofagus de la isla; fructifica en primavera y es objeto de recolección tradicional.',
        false, 'No Evaluada', 'fungi', 'Cyttaria',
        '{"tipo":"ascomiceto","comestibilidad":"comestible","simbiosis":"parasito","sustrato":["madera_viva"],"tipo_himenio":"liso","temporada":["primavera"],"advertencia":"Comestible y de consumo tradicional, pero recolectar solo ejemplares frescos y firmes: los cuerpos fructíferos pasados se descomponen rápido. Consulte a un experto antes de consumir cualquier hongo silvestre."}',
        '["Darwin, C. — Diario del viaje del Beagle","Micología aplicada — Hongos comestibles del bosque templado chileno"]'
    ),
    (
        'Grifola gargal', 'Gargal', 'Singer',
        'Basidiomiceto de gran tamaño que forma carpóforos compuestos por numerosas repisas superpuestas naciendo de una base común, con la superficie superior de color crema a pardo claro y el himenio de poros finos en la cara inferior. Es característico su aroma intenso y dulzón, comparado con el del anís o el almendro amargo, que persiste incluso en el ejemplar seco y que permite detectarlo antes de verlo. Crece sobre troncos vivos y muertos de Nothofagus, donde actúa como parásito de debilidad y luego como descomponedor. Es comestible y muy apreciado en la cocina del sur, y ha sido objeto de investigación para su cultivo controlado y para el estudio de sus compuestos aromáticos.',
        'Troncos vivos y muertos de Nothofagus en bosques templados maduros, generalmente en la base o en heridas del tronco.',
        'Presente en bosques de Nothofagus de la isla; fructifica en otoño y es recolectado localmente.',
        false, 'No Evaluada', 'fungi', 'Grifola',
        '{"tipo":"basidiomiceto","comestibilidad":"comestible","simbiosis":"parasito","sustrato":["madera_viva","madera_muerta"],"tipo_himenio":"poros","temporada":["otono"],"advertencia":"Comestible apreciado, pero su identificación requiere experiencia: consulte a un experto antes de consumir cualquier hongo silvestre."}',
        '["Micología aplicada — Hongos comestibles del bosque templado chileno","INFOR — Productos forestales no madereros del sur de Chile"]'
    ),
    (
        'Fistulina antarctica', 'Pinatra', 'Speg.',
        'Basidiomiceto de carpóforo carnoso, grueso y de color rosado a rojo intenso, con aspecto y consistencia que recuerdan a un trozo de carne, hasta el punto de que al cortarlo exuda un líquido rojizo. Su himenio es peculiar: en lugar de poros fusionados presenta tubos independientes y separados entre sí, carácter distintivo de la familia. Crece sobre el tronco de Nothofagus vivos, asociado a las agallas y deformaciones que provoca en el hospedero, y su presencia indica árboles con pudrición interna. Es comestible cuando está joven y firme, de sabor ácido, y forma parte del repertorio de hongos recolectados en el sur, aunque su uso es menos extendido que el del gargal o el digüeñe.',
        'Troncos vivos de Nothofagus, asociado a heridas y agallas del hospedero en bosques templados.',
        'Presente en bosques de Nothofagus de la isla; fructifica principalmente en otoño.',
        false, 'No Evaluada', 'fungi', 'Fistulina',
        '{"tipo":"basidiomiceto","comestibilidad":"comestible","simbiosis":"parasito","sustrato":["madera_viva"],"tipo_himenio":"poros","temporada":["otono"],"advertencia":"Comestible solo joven y firme; los ejemplares maduros se vuelven correosos y amargos. Consulte a un experto antes de consumir cualquier hongo silvestre."}',
        '["Micología aplicada — Hongos comestibles del bosque templado chileno","INFOR — Productos forestales no madereros del sur de Chile"]'
    ),
    (
        'Amanita muscaria', 'Matamoscas', '(L.) Lam.',
        'El hongo más reconocible del mundo: sombrero de un rojo escarlata intenso cubierto de verrugas blancas —restos del velo universal que lo envolvía—, láminas blancas libres, anillo membranoso y una base bulbosa con restos de volva. No es nativo de Chile: llegó junto con las plantaciones de pino y otras coníferas exóticas, con cuyas raíces forma micorrizas, y desde ellas se ha expandido, llegando a asociarse también con especies nativas, lo que genera preocupación por su efecto sobre las micorrizas propias del bosque templado. Contiene ácido iboténico y muscimol, compuestos neurotóxicos que provocan cuadros de confusión, delirio, náuseas y convulsiones. No es un hongo comestible bajo ninguna preparación casera.',
        'Bosques y plantaciones de coníferas exóticas, bordes de camino y parques; forma micorrizas con pinos, abedules y, crecientemente, especies nativas.',
        'Presente en plantaciones y bordes de camino de la isla, en expansión desde los sectores forestados hacia el bosque nativo.',
        false, 'No Evaluada', 'fungi', 'Amanita',
        '{"tipo":"agaricomiceto","comestibilidad":"toxico","simbiosis":"micorrizico","sustrato":["suelo","hojarasca"],"tipo_himenio":"laminas","temporada":["otono","invierno"],"advertencia":"TÓXICO Y PSICOACTIVO. Contiene ácido iboténico y muscimol: provoca confusión, delirio, vómitos y convulsiones. No existe preparación casera que lo vuelva seguro. Especie introducida que además desplaza micorrizas nativas. Consulte a un experto antes de consumir cualquier hongo silvestre."}',
        '["Ministerio de Salud de Chile — Intoxicaciones por hongos silvestres","Micología aplicada — Hongos introducidos en Chile"]'
    ),
    (
        'Pseudocyphellaria berberina', 'Liquen dorado', '(G.Forst.) D.J.Galloway & P.James',
        'Liquen foliáceo de gran tamaño que cuelga de troncos y ramas del bosque templado formando láminas lobuladas de color pardo verdoso en la cara superior y un envés tomentoso recorrido por pseudocifelas —poros diminutos por donde intercambia gases— de un amarillo intenso que le da el nombre. Como todo liquen es una simbiosis: un hongo que estructura el talo y un alga o cianobacteria que aporta los fotosintatos. Carece de raíces y absorbe agua y nutrientes directamente de la atmósfera, lo que lo hace extremadamente sensible a la contaminación del aire y lo convierte en bioindicador: su abundancia en los bosques de Chiloé refleja la excepcional pureza atmosférica de la zona.',
        'Troncos y ramas de árboles del bosque templado húmedo, con humedad atmosférica alta y constante.',
        'Abundante en los bosques húmedos de la isla, donde cuelga de ramas y troncos junto a musgos y otros líquenes.',
        false, 'No Evaluada', 'fungi', 'Pseudocyphellaria',
        '{"tipo":"liquen","comestibilidad":"no_comestible","simbiosis":"liquenizado","sustrato":["madera_viva","musgo"],"tipo_himenio":"liso","temporada":["todo_el_ano"],"advertencia":"No comestible. Los líquenes concentran metales pesados y contaminantes atmosféricos, y muchos contienen ácidos liquénicos irritantes."}',
        '["Galloway, D.J. — Flora liquenológica de Chile","Revista Chilena de Historia Natural — Líquenes como bioindicadores"]'
    ),
    (
        'Usnea barbata', 'Barba de viejo', '(L.) F.H.Wigg.',
        'Liquen fruticoso que cuelga de las ramas en mechones grisáceos o verdosos finamente ramificados, con el aspecto de una barba enmarañada que da nombre al conjunto de la vegetación colgante del bosque lluvioso. Su rasgo diagnóstico está en el interior: al estirar suavemente una rama se descubre un cordón central blanco y elástico que resiste la tracción, carácter exclusivo del género. Contiene ácido úsnico, un compuesto con actividad antibiótica documentada que sustentó su uso tradicional en el sur de Chile como cicatrizante y antiséptico para heridas. Es un bioindicador estricto de calidad del aire: desaparece de los primeros donde hay dióxido de azufre, y su abundancia en Chiloé señala una atmósfera limpia.',
        'Ramas de árboles y arbustos del bosque templado húmedo, especialmente donde la neblina y la lluvia son frecuentes.',
        'Muy abundante colgando de las ramas en los bosques húmedos de toda la isla.',
        false, 'No Evaluada', 'fungi', 'Usnea',
        '{"tipo":"liquen","comestibilidad":"no_comestible","simbiosis":"liquenizado","sustrato":["madera_viva","madera_muerta"],"tipo_himenio":"liso","temporada":["todo_el_ano"],"advertencia":"No comestible. El ácido úsnico es hepatotóxico en dosis altas: su uso medicinal tradicional es tópico, nunca por ingesta. Consulte a un profesional de salud."}',
        '["Galloway, D.J. — Flora liquenológica de Chile","Revista Chilena de Historia Natural — Líquenes como bioindicadores"]'
    )
) AS v(
    nombre_cientifico, nombre_comun, autor_cientifico, descripcion, habitat,
    distribucion_chiloe, endemica, estado_conservacion, reino, genero,
    atributos, fuentes
)
JOIN generos g ON g.nombre = v.genero
JOIN familias f ON f.id = g.familia_id AND f.reino = v.reino::reino_enum
ON CONFLICT (nombre_cientifico) DO NOTHING;

-- =============================================================================
-- PROTISTA
-- =============================================================================

INSERT INTO familias (nombre, reino, descripcion) VALUES
    ('Durvillaeaceae',  'protista', 'Algas pardas de talo coriáceo y estructura interna alveolar.'),
    ('Lessoniaceae',    'protista', 'Algas pardas de estipes rígidos formadoras de bosques submarinos.'),
    ('Gracilariaceae',  'protista', 'Algas rojas cilíndricas, principal fuente mundial de agar.')
ON CONFLICT (reino, nombre) DO NOTHING;

INSERT INTO generos (nombre, familia_id, descripcion)
SELECT v.nombre, f.id, v.descripcion
FROM (VALUES
    ('Durvillaea',  'protista', 'Durvillaeaceae', 'Cochayuyos, algas de la rompiente expuesta.'),
    ('Lessonia',    'protista', 'Lessoniaceae',   'Huiros negros, formadores de praderas submareales.'),
    ('Gracilaria',  'protista', 'Gracilariaceae', 'Pelillos, algas rojas cultivadas para agar.')
) AS v(nombre, reino, familia, descripcion)
JOIN familias f ON f.nombre = v.familia AND f.reino = v.reino::reino_enum
ON CONFLICT (familia_id, nombre) DO NOTHING;

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
        'Durvillaea antarctica', 'Cochayuyo', '(Chamisso) Hariot',
        'Alga parda de gran porte que vive fijada a la roca en la zona de rompiente más expuesta del litoral, donde soporta un embate de olas que ninguna estructura rígida resistiría. Su secreto está en la arquitectura interna del talo: un tejido alveolar en panal, lleno de cámaras de aire, que le da simultáneamente flotabilidad y una resistencia a la tracción excepcional, y en un disco de fijación que se adhiere a la roca con una fuerza extraordinaria. Sus frondas correosas, de varios metros, se cosechan, se secan al sol y se comercializan en trenzas; son un alimento tradicional del litoral chileno, rico en fibra, yodo, alginatos y minerales, base de charquicanes, ensaladas y guisos. Cuando la marejada arranca las plantas, estas viajan flotando cientos de kilómetros llevando consigo una comunidad entera de invertebrados adheridos.',
        'Intermareal bajo y submareal somero de costas rocosas expuestas al oleaje intenso.',
        'Presente en toda la costa expuesta occidental del archipiélago; su recolección y secado es una actividad tradicional.',
        false, 'No Evaluada', 'protista', 'Durvillaea',
        '{"grupo":"algas_pardas","ambiente":"marino","morfologia":"talo","tamano_promedio_mm":10000,"importancia_ecologica":"Forma cinturones en la rompiente que amortiguan el oleaje y dan refugio a una fauna asociada abundante. Al desprenderse flota a la deriva y actúa como balsa biológica, transportando invertebrados a lo largo del océano Austral y conectando poblaciones distantes. Es además un recurso de recolección de orilla con valor económico y alimentario tradicional en el litoral chileno."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Recursos algales","IFOP — Praderas de macroalgas del sur de Chile"]'
    ),
    (
        'Lessonia spicata', 'Huiro negro', '(Suhr) Santelices',
        'Alga parda de gran tamaño con un disco de fijación robusto del que nacen estipes rígidos y ramificados que sostienen frondas coriáceas en la punta, dándole al conjunto el aspecto de un arbusto submarino. Forma praderas densas en el submareal somero de costas expuestas, y esas praderas son un hábitat estructural comparable a un bosque terrestre: dan refugio, sustrato y alimento a peces juveniles, crustáceos, moluscos y erizos, y amortiguan la energía del oleaje sobre la costa. Es la principal fuente de alginatos de la industria chilena, extraída mediante barreteo del disco de fijación —una práctica destructiva porque impide el rebrote— o recolectada como alga varada, motivo de regulación y conflicto entre recolectores y autoridad pesquera.',
        'Submareal somero e intermareal bajo de costas rocosas expuestas, formando praderas densas sobre roca firme.',
        'Presente en la costa expuesta del archipiélago, donde forma praderas explotadas por recolectores de orilla.',
        false, 'No Evaluada', 'protista', 'Lessonia',
        '{"grupo":"algas_pardas","ambiente":"marino","morfologia":"talo","tamano_promedio_mm":2000,"importancia_ecologica":"Ingeniero ecosistémico: sus praderas crean hábitat tridimensional para juveniles de peces e invertebrados y reducen la energía del oleaje sobre la costa. Es la base de la industria nacional de alginatos, y la extracción por barreteo del disco de fijación impide el rebrote, por lo que su manejo determina la persistencia de las praderas."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Recursos algales","IFOP — Praderas de macroalgas del sur de Chile"]'
    ),
    (
        'Gracilaria chilensis', 'Pelillo', 'C.J.Bird, McLachlan & E.C.Oliveira',
        'Alga roja de talo cilíndrico, delgado y muy ramificado, de color pardo rojizo a violáceo, que crece sobre fondos blandos de arena y fango en bahías y estuarios protegidos. Es la macroalga cultivada más importante de Chile: se propaga vegetativamente enterrando fragmentos del talo en el sedimento, técnica sencilla que permitió el desarrollo de una actividad de cultivo de orilla practicada por familias de pescadores artesanales, con fuerte participación de mujeres en las caletas del sur. De ella se extrae el agar, polisacárido gelificante indispensable en microbiología como medio de cultivo y ampliamente usado en alimentos. Además, su cultivo cumple una función de biorremediación al absorber el exceso de nutrientes disueltos en zonas afectadas por la acuicultura intensiva.',
        'Fondos blandos de arena y fango en bahías, estuarios y zonas protegidas someras con buena luz.',
        'Cultivada y recolectada en bahías y estuarios protegidos del mar interior del archipiélago.',
        false, 'No Evaluada', 'protista', 'Gracilaria',
        '{"grupo":"algas_rojas","ambiente":"estuarino","morfologia":"talo","tamano_promedio_mm":400,"importancia_ecologica":"Principal fuente nacional de agar y una de las pocas macroalgas cultivadas a escala en Chile, con un cultivo de orilla de baja tecnología accesible a la pesca artesanal. Su capacidad de asimilar nitrógeno y fósforo disueltos le da además valor como biorremediador en zonas con carga de nutrientes de origen acuícola."}',
        '["Subsecretaría de Pesca y Acuicultura de Chile — Recursos algales","IFOP — Cultivo de Gracilaria en Chile"]'
    )
) AS v(
    nombre_cientifico, nombre_comun, autor_cientifico, descripcion, habitat,
    distribucion_chiloe, endemica, estado_conservacion, reino, genero,
    atributos, fuentes
)
JOIN generos g ON g.nombre = v.genero
JOIN familias f ON f.id = g.familia_id AND f.reino = v.reino::reino_enum
ON CONFLICT (nombre_cientifico) DO NOTHING;

-- =============================================================================
-- MONERA
-- =============================================================================

INSERT INTO familias (nombre, reino, descripcion) VALUES
    ('Piscirickettsiaceae', 'monera', 'Gammaproteobacterias, varias patógenas de peces.'),
    ('Vibrionaceae',        'monera', 'Bacterias marinas y estuarinas de forma curvada, varias patógenas.')
ON CONFLICT (reino, nombre) DO NOTHING;

INSERT INTO generos (nombre, familia_id, descripcion)
SELECT v.nombre, f.id, v.descripcion
FROM (VALUES
    ('Piscirickettsia', 'monera', 'Piscirickettsiaceae', 'Bacterias intracelulares patógenas de salmónidos.'),
    ('Vibrio',          'monera', 'Vibrionaceae',        'Bacterias marinas curvadas, varias de interés sanitario.')
) AS v(nombre, reino, familia, descripcion)
JOIN familias f ON f.nombre = v.familia AND f.reino = v.reino::reino_enum
ON CONFLICT (familia_id, nombre) DO NOTHING;

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
        'Piscirickettsia salmonis', 'Bacteria del SRS', 'Fryer et al. 1992',
        'Bacteria intracelular facultativa descrita en Chile a comienzos de los años noventa a partir de brotes en salmónidos de cultivo, y agente de la piscirickettsiosis o síndrome rickettsial del salmón, la enfermedad infecciosa de mayor impacto económico de la salmonicultura chilena. Es pleomorfa, gramnegativa y se multiplica dentro de vacuolas en las células del pez, lo que la protege de buena parte de la respuesta inmune y dificulta que los antibióticos la alcancen en concentración suficiente. Esa dificultad ha derivado en un uso intensivo de antimicrobianos en la industria, con las consecuencias asociadas de resistencia bacteriana y presencia de residuos en el ambiente marino. Su control es hoy el principal problema sanitario de la acuicultura del archipiélago y un asunto ambiental de discusión pública permanente.',
        'Ambiente marino y estuarino; se multiplica dentro de células de salmónidos y persiste en el agua y el sedimento de los centros de cultivo.',
        'Presente en los centros de cultivo de salmónidos del mar interior del archipiélago, donde constituye el principal desafío sanitario de la industria.',
        false, 'No Evaluada', 'monera', 'Piscirickettsia',
        '{"dominio":"bacteria","forma":"pleomorfo","gram":"negativo","metabolismo":{"fuente_energia":"heterotrofo","oxigeno":"aerobio_estricto"},"relevancia_chiloe":"Agente del síndrome rickettsial del salmón, la principal causa de mortalidad infecciosa en la salmonicultura chilena, industria central en la economía del archipiélago. Su carácter intracelular obliga a tratamientos antibióticos prolongados, lo que ha convertido el uso de antimicrobianos y la resistencia bacteriana asociada en un problema ambiental y sanitario de discusión permanente en la región."}',
        '["SERNAPESCA — Informe sanitario de la salmonicultura chilena","Journal of Fish Diseases — Piscirickettsia salmonis"]'
    ),
    (
        'Vibrio parahaemolyticus', 'Vibrio de los mariscos', '(Fujino et al. 1951) Sakazaki et al. 1963',
        'Bacteria marina gramnegativa de forma curvada en coma, móvil por un flagelo polar y halófila: requiere sal para crecer, de modo que su hábitat natural son las aguas costeras y estuarinas templadas y cálidas, donde vive libre o asociada al plancton y a los moluscos filtradores. La mayoría de las cepas ambientales son inofensivas, pero las que portan los genes de las hemolisinas termoestables causan gastroenteritis aguda con diarrea, dolor abdominal y vómitos tras el consumo de mariscos crudos o insuficientemente cocidos. En Chile provocó un brote extenso y sostenido a comienzos de los años dos mil, concentrado justamente en la región de Los Lagos y vinculado al consumo de bivalvos, episodio que reorganizó la vigilancia sanitaria de los mariscos y estableció el monitoreo permanente que hoy rige en el archipiélago.',
        'Aguas costeras y estuarinas templadas; asociada al plancton, al sedimento y a los tejidos de moluscos bivalvos filtradores.',
        'Presente en las aguas y bivalvos del mar interior del archipiélago; su vigilancia es permanente en la temporada estival.',
        false, 'No Evaluada', 'monera', 'Vibrio',
        '{"dominio":"bacteria","forma":"vibrio","gram":"negativo","metabolismo":{"fuente_energia":"heterotrofo","oxigeno":"anaerobio_facultativo"},"relevancia_chiloe":"Responsable de brotes de gastroenteritis asociados al consumo de mariscos crudos, con un episodio mayor en la región de Los Lagos a comienzos de los años dos mil. Su abundancia aumenta con la temperatura del agua, por lo que el riesgo se concentra en verano y crece con el calentamiento del océano. La cocción completa de los bivalvos y el respeto de la cadena de frío son las medidas preventivas efectivas."}',
        '["Instituto de Salud Pública de Chile — Vigilancia de Vibrio parahaemolyticus","Ministerio de Salud de Chile — Prevención de enfermedades transmitidas por mariscos"]'
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
