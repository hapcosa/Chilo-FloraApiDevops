-- =============================================================================
-- 0011_avistamientos_indice_espacial.sql
-- =============================================================================
-- Índice para el mapa de encuentros (Fase 9, PR 9).
--
-- `GET /api/v1/avistamientos/mapa` agrega por celda dentro de un bounding box,
-- y hasta ahora la única forma de resolver ese `WHERE geo_lat BETWEEN … AND
-- geo_lng BETWEEN …` era recorrer la tabla entera. El mapa se repinta con cada
-- desplazamiento, así que es la consulta más repetida que va a tener la API.
--
-- Parcial sobre lo que el mapa realmente mira: público y aprobado. Los
-- encuentros privados son la mayoría esperada y nunca entran a esta consulta,
-- así que dejarlos fuera mantiene el índice pequeño. Es el mismo criterio de
-- `idx_avistamientos_feed` (migración 0008).
--
-- Índice compuesto B-tree y no GiST: `geo_lat`/`geo_lng` son NUMERIC, no un
-- tipo geométrico, y este proyecto no tiene PostGIS. Para un rango sobre dos
-- columnas ordenadas alcanza; si algún día entra PostGIS, esto se reemplaza por
-- un GiST sobre `geography`, que es lo que sabe hacer distancias de verdad.
-- =============================================================================

BEGIN;

CREATE INDEX IF NOT EXISTS idx_avistamientos_mapa
    ON avistamientos (geo_lat, geo_lng)
    WHERE estado = 'aprobado' AND visibilidad = 'publico';

COMMIT;
