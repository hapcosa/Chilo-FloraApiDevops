# Prompt para la siguiente sesión

Copiá todo lo que sigue como primer mensaje de la sesión nueva.

---

Contexto: repo `Chilo-FloraApiDevops` (backend microservicios C++/Go/Nginx) con submódulo
`mobile/` (React Native bare CLI). Leé `CLAUDE.md` (raíz y `mobile/`) y `docs/PLAN_MAESTRO.md`
antes de tocar nada.

La **Fase 8b (identificación comunitaria)** está cerrada y sus cuatro deudas técnicas también:
la última era el **feed comunitario**, que se construyó en la sesión anterior. Hasta entonces
la única entrada a `AvistamientoDetailScreen` era Perfil → "Mis encuentros", así que nadie
podía identificar encuentros ajenos — la identificación comunitaria estaba implementada pero
era inalcanzable.

## Qué se hizo en la sesión anterior

| PR | Repo | Estado | Qué |
|----|------|--------|-----|
| #47 | backend | mergeado | `GET /api/v1/avistamientos`: feed de ajenos + `identificaciones_count` |
| #48 | backend | mergeado | `visibilidad` privado/público + `PATCH /{id}/compartir` |
| #49 | backend | mergeado | `foto_url` firmada en cada avistamiento |
| #24 | móvil | **abierto, checks verdes** | `FeedScreen` + pestaña "Comunidad" |

Detalle:

- **Feed de la API (#47).** Listado paginado, filtros `reino` y `grado_identificacion`, orden
  por `observado_en` descendente. `identificaciones_count` viene en cada fila para que la
  tarjeta no dispare una petición por encuentro.
- **Visibilidad (#48).** ADR #12 estaba decidido desde julio pero **nunca implementado**: el
  móvil ya llamaba a `/compartir`, un endpoint que no existía. Migración `0008`, enum
  `avistamiento_visibilidad_enum`, columna con default `privado` y backfill a `publico` de lo
  ya aprobado. Son **dos ejes distintos, decididos por gente distinta**: `visibilidad` la
  elige el autor, `estado` la moderación. Un privado ajeno no lo ve nadie, tampoco un
  moderador — nunca se ofreció a nadie. ADR #19.
- **`foto_url` (#49).** `avistamientos-fotos` es un bucket privado, así que `foto_key` sola
  no es mostrable desde el móvil. La API firma una URL GET al responder (SigV4 local, sin
  llamar a MinIO), válida `S3_PRESIGN_EXPIRES_SECONDS` (900 s). No se guarda: caduca. Si
  firmar falla, `foto_url` viene `null` y el resto de la respuesta sigue igual. ADR #20.
  Verificado contra un MinIO real: sin firma 403, firmada 200, firma manipulada 403.
- **Feed móvil (#24, pendiente de merge).** `FeedScreen` con filtros de reino y grado,
  pull-to-refresh, paginación por offset, foto desde `foto_url` con emoji de reino como
  placeholder. `ComunidadStackNavigator` y pestaña 👥 Comunidad entre Explorar y Guardados.

### Desviación consciente del feed móvil

**El feed no se cachea en SQLite**, saltándose la regla offline-first de `mobile/CLAUDE.md`.
Razón: `foto_url` caduca a los ~15 minutos, así que un feed guardado se vería sin imágenes —
justo lo que se viene a mirar. Sin red muestra error con botón de reintento. Las mutaciones
(identificar, retirar) se siguen encolando como siempre; esto solo afecta a la lectura.

## Qué hacer en esta sesión

1. **Mergear el PR #24 del móvil** (lo hace el humano) y después **bumpear el submódulo**
   `mobile` en el backend, en su propio PR.
2. **Desplegar.** Producción quedó sin actualizar con todo lo anterior: `especies-api` arrastra
   el fix de ISO 8601 (#42), el feed (#47), la visibilidad (#48) y `foto_url` (#49). El #48
   trae migración `0008`, así que hay que correr `especies-api-migrate` **antes** de levantar
   la API.
3. **Verificar en el dispositivo** todo lo acumulado (lista abajo).

## Pendiente de verificación

Nada de las dos últimas sesiones se probó en el dispositivo ni en producción:

1. **Feed de comunidad**: entrar en la pestaña Comunidad con el usuario de prueba y comprobar
   que aparecen encuentros ajenos con foto, que los filtros de reino y grado acotan, que el
   scroll pagina y que tocar una tarjeta abre el detalle y deja identificar.
2. **Visibilidad**: un encuentro recién creado **no** debe aparecer en el feed hasta que su
   autor lo comparta. Verificar también que el autor sí lo ve en "Mis encuentros".
3. **`foto_url`**: que las fotos del feed carguen de verdad desde el móvil (es una URL contra
   `S3_PUBLIC_ENDPOINT`; si ese endpoint no es alcanzable desde el teléfono, salen todas con
   placeholder).
4. **Cache de especies**: Perfil → "Sincronizar" y comprobar que el contador de la biblioteca
   baja de 17 a 13 (los del seed).
5. **Retiro offline**: modo avión, retirar una identificación propia (debe tacharse con
   "retirada, pendiente de enviar"), quitar el modo avión y comprobar que se envía sola.
6. **ISO 8601**: que las fechas se vean bien **sin** depender del parche de `fechas.ts`
   (mirar la respuesta cruda de la API).

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

### Producción (host `10.244.117.161`, por VPN)

Producción **se migró el 2026-08-14**: el host viejo `trader@10.244.19.205` (`traderbot`) es
ahora el entorno de **test**. Las tres piezas que no viven en el repo (env de prod,
credenciales del túnel, `CLOUDFLARED_UID/GID`) están documentadas en `CLAUDE.md`; sin ellas el
stack no arranca.

Mergear a `master` **no** despliega nada. Hay que reconstruir a mano:

```bash
cd infrastructure/docker

# Migraciones primero si el PR trae una (el #48 trae la 0008):
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up especies-api-migrate

# Reconstruir y levantar SOLO el servicio que cambió:
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml build especies-api
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up -d especies-api

docker compose -p chiloe-prod -f docker-compose.prod.yml ps
docker logs -f chiloe-especies-api
```

**Nombrá siempre el servicio concreto.** En el host de test conviven ~21 contenedores de otro
negocio (prefijos `budai_`, `wt-`); los nuestros llevan prefijo `chiloe-`. Un `up -d` a secas
es peligroso ahí.

Servicios del compose de prod: `postgres`, `redis`, `minio`, `minio-create-buckets`,
`especies-api-migrate`, `especies-api`, `auth-service`, `gateway`, `cloudflared`.

⚠️ **Nunca dos `cloudflared` del mismo túnel a la vez**: Cloudflare ve dos conectores y
reparte el tráfico entre ambas máquinas.

## Estado del entorno dev (ahorra media hora)

- **Cuidado con los contenedores de otros proyectos en este host.** No basta con que un puerto
  responda: lanzando un MinIO de prueba en el 59000, el `docker run` falló en silencio
  ("port is already allocated") y el `curl` de health devolvió 200 **desde el MinIO de otro
  proyecto**. Los `mc alias set` iban a la instancia ajena y solo fallaron porque las
  credenciales no coincidían. Antes de usar un puerto: `ss -ltn`, y después verificá que el
  contenedor está `Up` con los puertos publicados, no solo `Created`.
- **MinIO dev puede no levantar**: `dvu-minio-1` de otro proyecto ocupa el 9000. Si pasa, el
  alta de avistamientos vía API falla con "No se pudo consultar object storage".
- **Datos de prueba en la BD dev**: avistamiento `id=1` ("Arbol grande", `creado_por=2`,
  `estado='aprobado'`), sus identificaciones (una retirada, id 2) y el usuario
  `pruebag@chiloe.dev` (id 2, password `PruebaG2026!`). Credencial solo de desarrollo.
- **Postgres tarda en estar listo de verdad**: `pg_isready` responde OK contra el servidor
  temporal de la fase de init y las migraciones fallan con "the database system is shutting
  down". Esperá con `docker exec ... psql -c 'SELECT 1'`, no con `pg_isready`.
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
- `npm run lint` arrastra ~25 warnings `no-void` preexistentes en todo el repo. Lo que importa
  es que salga con **0 errores**.
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
