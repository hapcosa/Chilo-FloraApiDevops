# Chiloé Especies API

Microservicio REST en C++17 + Pistache para el catálogo multi-reino de la
biodiversidad de Chiloé.

Este servicio reemplaza el alcance antiguo de `flora-api`. Las decisiones de
modelo y arquitectura están en [../../docs/PLAN_MAESTRO.md](../../docs/PLAN_MAESTRO.md).

## Responsabilidades

- CRUD de familias, géneros y especies.
- Modelo multi-reino: `animalia`, `plantae`, `fungi`, `protista`, `monera`.
- Validación de `atributos_especificos` con JSON Schema por reino.
- Filtros y paginación para `GET /api/v1/especies` y ruta legacy
  `GET /api/especies`.
- Persistencia en PostgreSQL mediante migraciones SQL planas.

La infraestructura MinIO/S3 ya existe en Docker Compose dev y manifiestos K8s
base. El endpoint `POST /api/v1/uploads/presign` genera URLs `PUT` firmadas.
`PATCH /api/v1/especies/:id/fotos` guarda `foto_portada_key` y `fotos_keys`
solo si las keys existen en el bucket `especies-fotos`. Ese bucket queda con
descarga pública; `avistamientos-fotos` se mantiene privado.

El controlador todavía conserva endpoints legacy de imágenes binarias
(`/api/especies/:id/images`). No forman parte del flujo de producción definido
en el plan maestro.

## Desarrollo Con Docker Compose

Desde la raíz del repo:

```bash
make dev
make logs-especies
make api-test
```

URL local:

```bash
curl http://localhost:9081/health
```

El compose monta:

- `src/`
- `include/`
- `CMakeLists.txt`
- `config/schemas/`

Además define `SCHEMAS_DIR=/app/config/schemas`, necesario para que el servicio
cargue los JSON Schemas al iniciar.

## Migraciones

Las tablas no se crean desde el código de aplicación. El schema vive en:

```text
services/especies-api/migrations/
```

El runner:

```bash
services/especies-api/scripts/migrate.sh
```

usa la tabla `schema_migrations` para no reaplicar migraciones ya ejecutadas.

## Endpoints Principales

```text
GET    /health

POST   /api/v1/uploads/presign
PATCH  /api/v1/especies/{id}/fotos

GET    /api/familias
GET    /api/familias/{id}
POST   /api/familias
PUT    /api/familias/{id}
DELETE /api/familias/{id}

GET    /api/generos
GET    /api/generos/{id}
POST   /api/generos
PUT    /api/generos/{id}
DELETE /api/generos/{id}

GET    /api/especies
GET    /api/especies/{id}
POST   /api/especies
PUT    /api/especies/{id}
DELETE /api/especies/{id}
PATCH  /api/especies/{id}/fotos

GET    /api/v1/especies
GET    /api/v1/especies/{id}
POST   /api/v1/especies
PUT    /api/v1/especies/{id}
DELETE /api/v1/especies/{id}
PATCH  /api/v1/especies/{id}/fotos
POST   /api/v1/especies/{id}/publicar      (curaduría sobre la categoría de la ficha)
POST   /api/v1/especies/{id}/despublicar   (idem)

GET    /api/v1/avistamientos
GET    /api/v1/avistamientos/{id}
POST   /api/v1/avistamientos
PATCH  /api/v1/avistamientos/{id}/moderacion

GET    /api/v1/categorias            (?reino=fungi para filtrar)
GET    /api/v1/categorias/{id}
POST   /api/v1/categorias            (solo admin)
PUT    /api/v1/categorias/{id}       (solo admin)
DELETE /api/v1/categorias/{id}       (solo admin; 409 si tiene especies)

POST   /api/v1/categorias/{id}/moderadores/{usuarioId}   (solo admin)
DELETE /api/v1/categorias/{id}/moderadores/{usuarioId}   (solo admin)
GET    /api/v1/moderadores/{usuarioId}/categorias        (admin o el propio usuario)

POST   /api/v1/postulaciones          (cualquier sesión; postula a nombre propio)
GET    /api/v1/postulaciones          (admin: todas, ?estado=pendiente; resto: las suyas)
GET    /api/v1/postulaciones/{id}     (admin o el postulante)
PATCH  /api/v1/postulaciones/{id}     (solo admin; {"estado":"aprobada"} o
                                       {"estado":"rechazada","motivo":"..."})
```

Aprobar una postulación inserta la fila en `moderador_categorias` **en la misma
transacción**, y no toca los roles del `auth-service`: el curador sigue siendo rol
`user` con asignaciones.

Las mutaciones de especies (`POST`, `PUT`, `PATCH .../fotos`, `DELETE`) ya no exigen rol
`admin`/`moderator`: exigen **curaduría sobre la categoría de la ficha**. `admin` y
`moderator` siguen siendo globales; un usuario con rol `user` y una fila en
`moderador_categorias` puede editar solo lo de sus categorías. Ver
[../../docs/PLAN_MAESTRO.md](../../docs/PLAN_MAESTRO.md) §3.

Toda ficha nueva nace en estado **`borrador`** y solo se vuelve pública con
`POST /api/v1/especies/{id}/publicar`, que además revalida `atributos_especificos`
contra el JSON Schema del reino y guarda `publicado_por`/`fecha_publicacion`.
`estado` en el cuerpo de un `POST`/`PUT` se ignora: un `PUT` con datos viejos no
despublica. Los `GET` solo muestran borradores a quien tiene curaduría sobre esa
categoría (y a `admin`/`moderator`); a cualquier otro le responden **404**, no 403.

Filtros soportados en `GET /api/v1/especies` y `GET /api/especies`:

```text
reino
genero_id
familia_id
conservacion
endemica
estado          (borrador|publicada; se aplica dentro de lo que ya se puede ver)
q
limit
offset
orderby
orderdir
```

Ejemplo:

```bash
curl "http://localhost:9081/api/v1/especies?reino=plantae&q=canelo&limit=20&offset=0"
```

Ejemplo de presigned URL:

```bash
curl -X POST http://localhost:9081/api/v1/uploads/presign \
  -H "Content-Type: application/json" \
  -d '{"bucket":"especies-fotos","filename":"canelo.jpg","content_type":"image/jpeg"}'
```

Ejemplo para persistir keys ya subidas:

```bash
curl -X PATCH http://localhost:9081/api/v1/especies/1/fotos \
  -H "Content-Type: application/json" \
  -d '{"foto_portada_key":"especies/2026/07/15/abc-canelo.jpg","fotos_keys":["especies/2026/07/15/abc-canelo.jpg"]}'
```

Ejemplo para crear un avistamiento:

```bash
curl -X POST http://localhost:9081/api/v1/avistamientos \
  -H "Content-Type: application/json" \
  -d '{"reino":"plantae","foto_key":"avistamientos/2026/07/15/abc-canelo.jpg","geo_lat":-42.62,"geo_lng":-73.78,"nombre_sugerido":"Canelo"}'
```

## Tests

Con dependencias locales instaladas:

```bash
make cpp-test
```

Con Docker, el CI usa el stage `tester` del Dockerfile:

```bash
docker build --target tester -t especies-api-test services/especies-api
```

Los tests cubren serialización/parseo de `Reino`, validación real de JSON
Schemas por reino y generación/validación básica de presigned URLs.
