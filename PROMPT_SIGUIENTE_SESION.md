# Prompt para la siguiente sesión

Copiá todo lo que sigue como primer mensaje de la sesión nueva.

---

Contexto: repo `Chilo-FloraApiDevops` (backend microservicios C++/Go/Nginx) con submódulo
`mobile/` (React Native bare CLI). Leé `CLAUDE.md` (raíz y `mobile/`) y `docs/PLAN_MAESTRO.md`
antes de tocar nada.

La **Fase 8b (identificación comunitaria)** está cerrada. En la sesión anterior se saldaron
dos de las cuatro deudas técnicas que quedaron de esa fase.

## Qué se hizo en la sesión anterior (todo mergeado)

| PR | Repo | Qué |
|----|------|-----|
| #41 | backend | bump del submódulo `mobile` |
| #42 | backend | `fix(especies-api)`: serializar `TIMESTAMPTZ` en ISO 8601 |
| #43 | backend | bump del submódulo `mobile` |
| #22 | móvil | `fix(sync)`: reconciliar el cache SQLite de especies |
| #23 | móvil | `feat(sync)`: retirar una identificación sin red |
| #45 | backend | bump del submódulo `mobile` |

Detalle de lo que cambió:

- **ISO 8601 (deuda 1, cerrada).** `services/especies-api/include/utils/timestamps.hpp` +
  `src/utils/timestamps.cpp`: `utils::toIso8601` / `toIso8601Opt` normalizan lo que devuelve
  libpqxx (`2026-08-04 18:55:08.259598+00`) a `2026-08-04T18:55:08.259Z`. Se aplica en el
  mapeo de filas de los 8 repositorios, sin tocar el SQL. 12 tests en
  `tests/test_timestamps.cpp`. ADR #16 en `docs/PLAN_MAESTRO.md §10`.
  El parche del cliente (`mobile/src/utils/fechas.ts`) se dejó a propósito: sigue haciendo
  falta para respuestas viejas cacheadas en SQLite.
- **Cache SQLite (deuda 4, cerrada).** `pruneSpeciesNotIn` en `mobile/src/db/speciesCache.ts`,
  llamado desde `initialSync` solo si el barrido cubrió el total. Ojo: el diagnóstico original
  ("especies duplicadas") era incorrecto — `upsertSpecies` usa `ON CONFLICT(id) DO UPDATE`, así
  que un id nunca se duplica. Lo que pasaba es que el sync **nunca borraba**, así que
  sobrevivían especies de un seed anterior con otros ids.
- **Retiro offline (deuda 2, cerrada).** Tipo `retirar_identificacion` en `mutation_queue`,
  id de fila derivado del id remoto (`retiro-identificacion-<id>`) con `ON CONFLICT DO UPDATE`
  para no encolar dos DELETE. 404 y 409 del servidor cuentan como retiro cumplido; 403 es
  rechazo definitivo. La tarjeta se tacha apenas se encola.

## Qué hacer en esta sesión

**El servidor de producción está caído.** Avanzá todo lo que no dependa de él: código del
móvil y de la API, con sus tests. El deploy queda para cuando vuelva.

La única deuda grande que sobra es el **feed comunitario**: la única entrada a
`AvistamientoDetailScreen` es Perfil → "Mis encuentros", así que un usuario no puede
identificar avistamientos ajenos — que es justo el sentido de la identificación comunitaria.

Propuesta de troceo (un PR por paso, **confirmá el alcance conmigo antes de arrancar el 1**):

1. **API — listado de avistamientos ajenos.** `GET /api/v1/avistamientos` con paginación y
   filtros por `reino` y `grado_identificacion`, ordenado por `observado_en` descendente,
   devolviendo solo los `estado = 'aprobado'`. Mirá si ya existe algo parcial en
   `services/especies-api/src/controllers/avistamiento_controller.cpp` antes de añadir.
   Probablemente haga falta un índice por migración numerada nueva (**no** editar las
   existentes). Tests en `services/especies-api/tests/`.
2. **API — conteo de identificaciones en el listado.** Para que la tarjeta del feed muestre
   "3 identificaciones" sin una petición por fila.
3. **Móvil — pantalla de feed** con navegación a `AvistamientoDetailScreen`, tirando del
   cache SQLite cuando no hay red.
4. **Móvil — entrada al feed** desde la navegación principal.

Nota: se auditó `auth-service` (Go) buscando el bug de timestamps del PR #42: **no lo tiene**.
Usa `time.Time` plano y `encoding/json` lo serializa en RFC 3339. Nada que hacer ahí.

## Pendiente de verificación

Nada de lo de la sesión anterior se probó en el dispositivo ni en producción. El A53 está
conectado por `adb`, así que esto se puede verificar contra el entorno local (`make dev`):

1. **Cache de especies**: Perfil → "Sincronizar" y comprobar que el contador de la biblioteca
   baja de 17 a 13 (los del seed).
2. **Retiro offline**: poner el teléfono en modo avión, retirar una identificación propia
   (debe tacharse con "retirada, pendiente de enviar"), sacar el modo avión y comprobar que
   se envía sola y el listado del servidor la muestra retirada.
3. **ISO 8601**: que las fechas de los avistamientos e identificaciones se vean bien **sin**
   depender del parche de `fechas.ts` (mirar la respuesta cruda de la API).

## Cómo levantar los servicios

### Entorno local / test

```bash
cd /home/obrero/programacion/Chilo-FloraApiDevops
make dev          # levanta postgres, redis, minio, especies-api, auth-service, gateway
make ps           # estado
make logs         # logs de todo (o make logs-especies / logs-auth / logs-gateway)
make api-test     # health checks de gateway, auth, especies-api y minio
make dev-down     # bajar (sin borrar volúmenes)
```

Por debajo es `docker compose -f infrastructure/docker/docker-compose.dev.yml up -d`.
`make dev-full` añade pgadmin, prometheus y grafana (perfiles `tools` y `monitoring`).

Puertos: gateway `8080`, auth `8081`, especies-api `9081`, minio `9000`.

Ojo, el `Makefile` **no tiene** los targets `test`, `health-check`, `minikube-deploy` ni
`db-shell` que menciona `CLAUDE.md`. Los reales son `api-test`, `go-test`, `cpp-test`,
`exec-db`. `CLAUDE.md` está desactualizado en esa sección.

### Producción (host `trader@10.244.19.205`, por VPN)

Mergear a `master` **no** despliega nada. Hay que reconstruir a mano:

```bash
ssh trader@10.244.19.205
cd /home/trader/Proyectos/chiloe-biodiversidad-api
git pull --ff-only
cd infrastructure/docker

# Reconstruir y levantar SOLO el servicio que cambió:
docker compose -p chiloe-prod \
  --env-file /home/trader/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml build especies-api

docker compose -p chiloe-prod \
  --env-file /home/trader/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up -d especies-api

# Estado y logs:
docker compose -p chiloe-prod -f docker-compose.prod.yml ps
docker logs -f chiloe-especies-api
```

**Nombrá siempre el servicio concreto.** En ese host conviven ~21 contenedores de otro negocio
(prefijos `budai_`, `wt-`); los nuestros llevan prefijo `chiloe-`. Un `up -d` a secas es
peligroso ahí.

Servicios del compose de prod: `postgres`, `redis`, `minio`, `minio-create-buckets`,
`especies-api-migrate`, `especies-api`, `auth-service`, `gateway`, `cloudflared`.

El despliegue que falta es el de **`especies-api`** (por el PR #42, ISO 8601). Prod sale a
internet por Cloudflare Tunnel saliente, no por A-record.

## Estado del entorno dev (ahorra media hora)

- **MinIO dev puede no levantar**: el contenedor `dvu-minio-1` de otro proyecto ocupa el
  puerto 9000. Si pasa, el alta de avistamientos vía API falla con "No se pudo consultar
  object storage".
- **Datos de prueba en la BD dev**: avistamiento `id=1` ("Arbol grande", `creado_por=2`,
  `estado='aprobado'`), sus identificaciones (una retirada, id 2) y el usuario
  `pruebag@chiloe.dev` (id 2, password `PruebaG2026!`). Credencial solo de desarrollo.
- `psql`: el usuario es `$POSTGRES_USER` (`dev_user`), no `chiloe_user`. Usá
  `docker exec chiloe-postgres-dev sh -c 'psql -U "$POSTGRES_USER" -d ...'`.
- La tabla se llama `avistamiento_identificaciones`, no `identificaciones`.

## Build del móvil

- `JAVA_HOME=/usr/lib/jvm/java-17-openjdk` — el JDK 26 del sistema rompe el plugin de Kotlin.
- Metro en el puerto **8082**: `chiloe-auth-dev` ocupa el 8081 en el host.
- Dispositivo: `adb connect 192.168.1.4:36413` + `adb reverse tcp:8082 tcp:8082` y el reverse
  del gateway.
- **No corras `npx prettier` en el repo móvil**: no hay `.prettierrc`, así que usa defaults
  (comillas dobles) y reformatea archivos enteros fuera del estilo del proyecto. El formato
  se valida con `npm run lint`.
- `especies-api` **no compila en este host**: faltan Pistache, libpqxx y
  json-schema-validator. Para compilar o testear:
  `docker build --target tester -t especies-api-test services/especies-api`.
  La lógica pura (como `timestamps.cpp`) sí se puede compilar suelta con g++ + gtest.

## Reglas que no se negocian

- Rama por cambio → commits → push → PR contra `master` → checks verdes → **el merge lo hago yo**.
- Nada de push a `master`, `--force`, `reset --hard` ni `--no-verify`.
- Migraciones numeradas y **nunca** editadas tras mergear.
- No commitear `.idea/editor.xml`, `.idea/vcs.xml` ni el directorio sin trackear `diseño/`
  (submódulo móvil).
- Este archivo **sí** está trackeado: actualizalo al cerrar la sesión, en su propio PR.
- Avisame explícitamente cada desviación, default elegido u omisión. En español, directo.
