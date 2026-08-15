# Biodiversidad de Chiloé - Backend de Microservicios

Backend para una plataforma de divulgación científica sobre la biodiversidad de
Chiloé en los cinco reinos: Animalia, Plantae, Fungi, Protista y Monera.

El plan vivo del proyecto está en [docs/PLAN_MAESTRO.md](docs/PLAN_MAESTRO.md).
Ese documento manda sobre decisiones de arquitectura, fases y alcance.

## Servicios

| Servicio | Tecnología | Puerto dev | Carpeta |
| --- | --- | ---: | --- |
| Gateway | Nginx | 8080 | `services/gateway/` |
| Panel de curaduría | React + Vite + TypeScript (estático, servido por el gateway en `/curaduria/`) | 8080 | `services/panel-curaduria/` |
| Especies API | C++17 + Pistache + libpqxx | 9081 | `services/especies-api/` |
| Auth Service | Go + Gin + JWT | 8081 | `services/auth-service/` |
| PostgreSQL | PostgreSQL 15 | 5432 | `infrastructure/docker/` |
| Redis | Redis 7 | 6379 | `infrastructure/docker/` |
| MinIO | S3-compatible object storage | 9000 / 9001 | `infrastructure/docker/` |

## Estado Actual

- Fase 1 multi-reino cerrada: `especies-api`, migraciones SQL, `reino_enum`,
  `atributos_especificos JSONB`, JSON Schemas por reino, filtros y paginación.
- Auth local/JWT operativo. Existe flujo OAuth web por callback y login móvil
  `POST /api/v1/auth/google` con `id_token` verificado contra Google.
- Fase 2 de fotos cerrada: MinIO está configurado en Docker Compose dev y en
  manifiestos K8s base. `POST /api/v1/uploads/presign` ya genera presigned
  URLs PUT y `PATCH /api/v1/especies/{id}/fotos` valida keys contra object
  storage antes de guardarlas. `especies-fotos` queda con lectura pública y
  `avistamientos-fotos` restringido.
- Fase 4 iniciada: `mobile/` contiene scaffold React Native bare Android,
  pantallas base, cliente API/JWT y cache SQLite. El submódulo remoto sigue
  pendiente de inicializar/publicar porque el entorno actual no clona ni pushea;
  el Gradle wrapper debe generarse/verificarse en el repo móvil definitivo.

## Requisitos

- Docker
- Docker Compose v2 (`docker compose`)
- Make
- Git

Para compilar fuera de Docker necesitas además las dependencias nativas de cada
servicio. El flujo recomendado es Docker Compose.

## Inicio Rápido

```bash
make setup
make dev
make ps
make api-test
```

URLs locales:

- Gateway: <http://localhost:8080>
- Especies API: <http://localhost:9081>
- Auth Service: <http://localhost:8081>
- MinIO API: <http://localhost:9000>
- MinIO Console: <http://localhost:9001>
- PgAdmin, con profile tools: <http://localhost:8889>

Para detener:

```bash
make dev-down
```

Para eliminar también volúmenes de desarrollo:

```bash
make dev-down-volumes
```

## Comandos Útiles

```bash
make help             # Ver comandos disponibles
make dev              # Levantar servicios base
make dev-full         # Levantar servicios + herramientas + monitoreo
make logs             # Logs de todos los servicios
make logs-especies    # Logs de especies-api
make logs-auth        # Logs de auth-service
make exec-db          # Entrar a PostgreSQL
make go-test          # Tests del auth-service
make cpp-test         # Tests C++ si las dependencias locales están instaladas
make db-reset         # Recrear volumen de PostgreSQL dev
```

## Base de Datos

`especies-api` usa migraciones SQL planas:

- `services/especies-api/migrations/0001_initial.sql`
- `services/especies-api/migrations/0002_multi_reino.sql`

El runner `services/especies-api/scripts/migrate.sh` aplica las migraciones antes
de arrancar `especies-api` en Docker Compose.

`auth-service` actualmente usa `gorm.AutoMigrate`. Si esto cambia, debe quedar
registrado en el plan maestro.

## API Principal

Rutas de catálogo:

- `GET /api/especies`
- `GET /api/especies?reino=plantae&q=canelo&limit=20&offset=0`
- `GET /api/v1/especies`
- `GET /api/v1/especies/{id}`
- `POST /api/especies`
- `POST /api/v1/especies`
- `PUT /api/especies/{id}`
- `PUT /api/v1/especies/{id}`
- `DELETE /api/especies/{id}`
- `DELETE /api/v1/especies/{id}`
- `GET /api/familias`
- `GET /api/generos`
- `POST /api/v1/uploads/presign`
- `PATCH /api/v1/especies/{id}/fotos`
- `GET /api/v1/avistamientos?reino=fungi&grado_identificacion=en_discusion&limit=20&offset=0`
- `GET /api/v1/avistamientos/{id}`
- `POST /api/v1/avistamientos`
- `PATCH /api/v1/avistamientos/{id}/moderacion`
- `PATCH /api/v1/avistamientos/{id}/compartir`

El listado de avistamientos ordena por `observado_en` descendente y filtra por dos ejes
que decide gente distinta:

- **`visibilidad`** (`privado` | `publico`) la elige el autor. Un avistamiento nace
  `privado`; `PATCH /{id}/compartir` lo pone en `publico`. Los privados ajenos no los ve
  nadie más, tampoco la moderación: nunca se ofrecieron a nadie.
- **`estado`** lo decide la moderación. Dentro de lo público, admin/moderator ven
  cualquier estado y el resto solo `aprobado`.

Quien filtra por sus propios avistamientos (`creado_por` = su id) los ve enteros, privados
y sin aprobar incluidos: es la pantalla "Mis encuentros". Cada fila trae
`identificaciones_count`, las identificaciones vigentes de ese avistamiento.

Rutas de auth:

- `POST /api/v1/auth/register`
- `POST /api/v1/auth/login`
- `POST /api/v1/auth/google`
- `POST /api/v1/auth/refresh`
- `GET /api/v1/auth/google`
- `GET /api/v1/auth/whoami`

## Fotos

El modelo actual ya tiene `foto_portada_key` y `fotos_keys`. MinIO se levanta en
desarrollo con los buckets `especies-fotos` y `avistamientos-fotos`, y la API ya
genera presigned URLs para `PUT` directo al bucket. Al persistirlas, valida que
las keys existan en `especies-fotos`. La política anónima permite descarga
pública solo en `especies-fotos`; `avistamientos-fotos` queda privado.

El flujo definido para Fase 2 es:

1. El cliente pide una URL presigned.
2. El cliente sube directo a MinIO/S3.
3. El cliente informa la key a `especies-api`.
4. La API valida la key antes de guardarla con `PATCH /api/v1/especies/{id}/fotos`.

## Flujo de Trabajo

No se trabaja directo en `master`.

```bash
git checkout -b feat/descripcion-corta
# cambios
make go-test
make cpp-test
git push -u origin feat/descripcion-corta
# abrir PR contra master
```

No uses `git push --force`, `git reset --hard` ni `--no-verify` sin permiso
explícito.
