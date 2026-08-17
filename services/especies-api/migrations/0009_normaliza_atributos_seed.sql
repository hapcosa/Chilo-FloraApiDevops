-- =============================================================================
-- 0009_normaliza_atributos_seed.sql
-- =============================================================================
-- Corrige seis filas del seed 0001 cuyos `atributos_especificos` violan el JSON
-- Schema de su propio reino (services/especies-api/config/schemas/).
--
-- El seed escribe directo a Postgres, así que la validación de la API nunca se
-- ejecutó sobre esas filas y los valores inválidos entraron sin resistencia:
--
--   * `usos_tradicionales` (plantae) es un enum cerrado —medicinal, alimentario,
--     maderable, ornamental, tintoreo, fibra, ceremonial, forraje— pero cuatro
--     especies traen frases libres ("Tejuelas para revestimiento de casas e
--     iglesias").
--   * `sustrato` (fungi) es otro enum cerrado y dos especies traen descripciones.
--   * `temporada` (fungi) admite 'otono' sin tilde; Ramaria flava trae 'otoño'.
--
-- Consecuencia práctica: cualquier PUT sobre una de esas especies falla la
-- validación de esquema, aunque el cliente no haya tocado los atributos. La
-- ficha se vuelve inmodificable desde la API.
--
-- El seed 0001 ya quedó corregido en el mismo PR, pero eso solo sirve para
-- bases nuevas: sus INSERT llevan ON CONFLICT DO NOTHING y no actualizan las
-- filas existentes. Esta migración es la que lleva la corrección a las bases ya
-- pobladas, tal como exige seeds/README.md.
--
-- Idempotente: filtra por el valor defectuoso, así que reaplicarla no hace nada.
-- Filtra además por nombre_cientifico para no tocar fichas editadas a mano que
-- casualmente compartan un valor.
-- =============================================================================

BEGIN;

UPDATE especies
SET atributos_especificos =
        jsonb_set(atributos_especificos, '{usos_tradicionales}',
                  '["maderable","ceremonial"]'::jsonb)
WHERE nombre_cientifico = 'Fitzroya cupressoides'
  AND atributos_especificos -> 'usos_tradicionales'
      ? 'Tejuelas para revestimiento de casas e iglesias';

UPDATE especies
SET atributos_especificos =
        jsonb_set(atributos_especificos, '{usos_tradicionales}',
                  '["ceremonial","medicinal"]'::jsonb)
WHERE nombre_cientifico = 'Drimys winteri'
  AND atributos_especificos -> 'usos_tradicionales'
      ? 'Planta ceremonial mapuche-huilliche';

UPDATE especies
SET atributos_especificos =
        jsonb_set(atributos_especificos, '{usos_tradicionales}',
                  '["alimentario","fibra"]'::jsonb)
WHERE nombre_cientifico = 'Gunnera tinctoria'
  AND atributos_especificos -> 'usos_tradicionales'
      ? 'Pecíolo comestible crudo o en ensaladas';

UPDATE especies
SET atributos_especificos =
        jsonb_set(atributos_especificos, '{usos_tradicionales}',
                  '["alimentario","maderable"]'::jsonb)
WHERE nombre_cientifico = 'Luma apiculata'
  AND atributos_especificos -> 'usos_tradicionales'
      ? 'Frutos comestibles frescos o en mermelada';

UPDATE especies
SET atributos_especificos =
        jsonb_set(atributos_especificos, '{sustrato}', '["madera_viva"]'::jsonb)
WHERE nombre_cientifico = 'Cyttaria espinosae'
  AND atributos_especificos -> 'sustrato' ? 'Ramas vivas de Nothofagus dombeyi';

UPDATE especies
SET atributos_especificos =
        jsonb_set(
            jsonb_set(atributos_especificos, '{sustrato}',
                      '["suelo","hojarasca"]'::jsonb),
            '{temporada}', '["otono"]'::jsonb)
WHERE nombre_cientifico = 'Ramaria flava'
  AND atributos_especificos -> 'sustrato' ? 'Mantillo y suelo de bosque templado';

COMMIT;
