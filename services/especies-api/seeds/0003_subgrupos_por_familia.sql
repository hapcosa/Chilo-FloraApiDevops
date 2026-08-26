-- =============================================================================
-- 0003_subgrupos_por_familia.sql
-- =============================================================================
-- Clasifica en su subgrupo de navegación las fichas que todavía no lo tienen,
-- usando el mapeo familia → subgrupo que dejó la migración 0013.
--
-- Existe porque las migraciones corren antes que los seeds: el backfill de la
-- 0013 alcanza a las fichas de un entorno ya poblado (producción), pero en una
-- BD nueva las fichas se insertan después y quedarían con `categoria_id` nulo.
--
-- Idempotente y no destructivo: solo toca filas sin categoría, así que una
-- reclasificación hecha a mano por un admin no se pisa al reaplicar el seed.
-- =============================================================================

UPDATE especies e
SET categoria_id = fs.categoria_id
FROM generos g
JOIN familias f          ON f.id = g.familia_id
JOIN familia_subgrupo fs ON fs.familia = f.nombre
JOIN categorias_moderacion c ON c.id = fs.categoria_id
WHERE e.genero_id = g.id
  AND e.categoria_id IS NULL
  AND c.reino = e.reino;

-- Red de contención: lo que ninguna familia conocida clasifica cae en la
-- categoría "general" de su reino. Cubre monera, que a propósito no tiene
-- subgrupos, y cualquier familia nueva que todavía no esté en el mapeo.
--
-- Es también el hueco que dejaba la 0004 en una BD nueva: su backfill corría
-- antes de que los seeds insertaran las fichas, así que en un entorno recién
-- creado quedaban todas sin categoría. En producción ya están clasificadas y
-- este UPDATE no toca nada.
UPDATE especies e
SET categoria_id = c.id
FROM categorias_moderacion c
WHERE e.categoria_id IS NULL
  AND c.reino = e.reino
  AND c.slug = e.reino::text || '-general';
