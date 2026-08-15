# Prompt para la siguiente sesión

Copiá todo lo que sigue como primer mensaje de la sesión nueva.

---

Contexto: repo `Chilo-FloraApiDevops` (backend microservicios C++/Go/Nginx) con submódulo
`mobile/` (React Native bare CLI). Leé `CLAUDE.md` (raíz y `mobile/`) y `docs/PLAN_MAESTRO.md`
antes de tocar nada.

**Todo el código de la Fase 8b está en `master` y ninguna de sus piezas está desplegada.**
El objetivo de esta sesión es uno solo: **poner producción al día y dejar la app móvil
hablando con la API de producción de punta a punta**, verificado en el dispositivo. No hay
features nuevas pendientes.

El servidor de producción **ya está encendido** (`https://api.budaicapital.com/health` → 200,
`https://storage.budaicapital.com/minio/health/live` → 200).

## Lo que falta desplegar

`especies-api` en producción arrastra cuatro PRs mergeados y nunca desplegados:

| PR | Qué | Riesgo si falta |
|----|-----|-----------------|
| #42 | timestamps en ISO 8601 | fechas inválidas en el móvil salvo por el parche de `fechas.ts` |
| #47 | `GET /api/v1/avistamientos` (feed + `identificaciones_count`) | **la pestaña Comunidad da 404** |
| #48 | `visibilidad` + `PATCH /{id}/compartir` (**migración `0008`**) | el móvil llama a un endpoint que no existe |
| #49 | `foto_url` firmada | el feed sale entero con placeholders de emoji |

El gateway y `auth-service` no cambiaron; solo hace falta reconstruir `especies-api`.

## Paso 1 — desplegar

El host de producción es `10.244.117.161` (por VPN), migrado el 2026-08-14. **Confirmá primero
el usuario y la ruta del checkout en ese host**: `CLAUDE.md` documenta el comando de compose y
que el env vive en `~/.config/chiloe-prod/chiloe.env`, pero no la ruta del repo — en el host
viejo era `/home/trader/Proyectos/chiloe-biodiversidad-api`. Anotalo en este archivo cuando lo
sepas.

```bash
# en el host de prod, dentro del checkout
git pull --ff-only
git submodule update --init --recursive   # el puntero de mobile subió con el PR #51
cd infrastructure/docker

# 1. Migración 0008 ANTES de levantar la API nueva.
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up especies-api-migrate

# 2. Reconstruir y levantar SOLO especies-api.
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml build especies-api
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up -d especies-api

docker compose -p chiloe-prod -f docker-compose.prod.yml ps
docker logs -f chiloe-especies-api
```

**Nombrá siempre el servicio concreto.** En el host de test conviven ~21 contenedores de otro
negocio (prefijos `budai_`, `wt-`); los nuestros llevan prefijo `chiloe-`. Un `up -d` a secas
es peligroso ahí. Y **nunca dos `cloudflared` del mismo túnel a la vez**: Cloudflare ve dos
conectores y reparte el tráfico entre las dos máquinas.

Comprobaciones tras el deploy, desde cualquier máquina:

```bash
curl -s https://api.budaicapital.com/health                     # 200
curl -s -i "https://api.budaicapital.com/api/v1/avistamientos?limit=1"   # 401 sin token
# con un JWT de prod:
curl -s -H "Authorization: Bearer $TOKEN" \
  "https://api.budaicapital.com/api/v1/avistamientos?limit=5" | jq '.data[0]'
```

En esa respuesta mirá tres cosas: que exista `identificaciones_count`, que exista
`visibilidad`, y que `foto_url` **no** sea `null` cuando hay `foto_key`.

En la BD, confirmá que la migración corrió:

```sql
SELECT version FROM schema_migrations ORDER BY version DESC LIMIT 3;
SELECT visibilidad, count(*) FROM avistamientos GROUP BY 1;
```

El backfill de la `0008` puso en `publico` lo que ya estaba `aprobado`; el resto queda
`privado`. Si todo sale `privado`, el feed se verá vacío y **no es un bug**.

## Paso 2 — el cableado del móvil (el punto delicado)

La URL ya está en el código, no hay nada que inventar:
`mobile/src/config/appConfig.ts` usa `http://localhost:8080` en debug (con `adb reverse`) y
`https://api.budaicapital.com` en release. Lo que hay que verificar es la cadena entera:

1. **`S3_PUBLIC_ENDPOINT` en `~/.config/chiloe-prod/chiloe.env` debe ser
   `https://storage.budaicapital.com`**, no `http://minio:9000`. `foto_url` se firma con ese
   host **dentro de la firma SigV4**: si apunta a un nombre interno, la URL no resuelve desde
   el teléfono, y si se cambia el host después de firmar, MinIO responde 403. El túnel ya
   tiene el hostname publicado (`infrastructure/docker/cloudflared/config.yml`), porque el
   cliente sube las fotos **directo** a MinIO y nunca pasan por el gateway.
2. **Build release y probarlo sin Metro ni `adb reverse`**, que es la única forma de comprobar
   que la app habla con producción y no con el túnel de desarrollo:
   ```bash
   cd mobile
   JAVA_HOME=/usr/lib/jvm/java-17-openjdk ./android/gradlew -p android assembleRelease
   adb install -r android/app/build/outputs/apk/release/app-release.apk
   ```
   Antes de probar, **quitá los `adb reverse`** (`adb reverse --remove-all`) para que no haya
   forma de que el tráfico se cuele al backend local.
3. **Google Sign-In en release**: hoy `android/app/build.gradle` firma el release con el
   **keystore de debug** (`signingConfig signingConfigs.debug` dentro de `buildTypes.release`).
   Eso hace que el SHA-1 sea el mismo que en debug, así que el login sigue funcionando — pero
   es inaceptable para publicar. Si en esta sesión se decide crear un keystore propio, hay que
   registrar su SHA-1 en Google Cloud **antes**, o el login falla con `DEVELOPER_ERROR`. El
   keystore y sus contraseñas **no se commitean**.

## Paso 3 — verificación en el dispositivo

El A53 se conecta con `adb connect 192.168.1.4:36413`. Con el APK release instalado y **sin**
red local hacia el backend de desarrollo:

1. **Login** con `pruebag@chiloe.dev` (si ese usuario existe en la BD de prod; si no, registrar
   uno) y con Google.
2. **Feed de comunidad**: la pestaña 👥 muestra encuentros ajenos **con foto**. Si salen todos
   con emoji de placeholder, el problema es `S3_PUBLIC_ENDPOINT` o el túnel de storage, no el
   móvil.
3. **Filtros y paginación**: acotar por reino y por grado, y hacer scroll hasta que pida la
   segunda página.
4. **Identificar** un encuentro ajeno desde el feed y ver que el contador sube.
5. **Visibilidad**: crear un encuentro nuevo y comprobar que **no** aparece en el feed hasta
   compartirlo, y que sí aparece en Perfil → "Mis encuentros".
6. **Subida de foto**: sacar una foto y ver que el PUT presignado a `storage.budaicapital.com`
   funciona (es el mismo camino que la `foto_url` de lectura, pero de escritura).
7. **Retiro offline**: modo avión, retirar una identificación propia (se tacha con "pendiente
   de enviar"), quitar el modo avión y ver que se envía sola.
8. **ISO 8601**: mirar la respuesta cruda de la API y confirmar que las fechas ya vienen
   normalizadas sin depender del parche de `mobile/src/utils/fechas.ts`.
9. **Cache de especies**: Perfil → "Sincronizar" y ver que el contador de la biblioteca baja a
   los del seed.

## Si algo falla

- **El feed da 404** → el deploy de `especies-api` no tomó; mirá `docker logs chiloe-especies-api`
  y confirmá que la imagen se reconstruyó (no basta con `up -d`, hace falta `build`).
- **El feed sale vacío pero responde 200** → todos los avistamientos quedaron `privado`
  (comportamiento correcto de la `0008`). Compartí uno desde el móvil o con
  `PATCH /api/v1/avistamientos/{id}/compartir`.
- **Las fotos no cargan** → `S3_PUBLIC_ENDPOINT`. Verificá con
  `curl -I "<foto_url copiada de la respuesta>"`: 200 es correcto, 403 significa que el host de
  la firma no coincide con el host al que se pide.
- **Login Google con `DEVELOPER_ERROR`** → SHA-1 del keystore no registrado en Google Cloud.

## Entorno local (por si hace falta reproducir algo)

```bash
cd /home/obrero/programacion/Chilo-FloraApiDevops
make dev          # postgres, redis, minio, especies-api, auth-service, gateway
make ps / make logs / make api-test / make dev-down
```

Puertos: gateway `8080`, auth `8081`, especies-api `9081`, minio `9000`. El `Makefile` **no
tiene** los targets `test`, `health-check`, `minikube-deploy` ni `db-shell` que menciona
`CLAUDE.md`; los reales son `api-test`, `go-test`, `cpp-test`, `exec-db`.

Trampas del host de desarrollo que ya costaron tiempo:

- **Contenedores de otros proyectos**: que un puerto responda no significa que sea tuyo. Un
  `docker run` falló en silencio ("port is already allocated") y el health check devolvió 200
  **desde el MinIO de otro proyecto**. Antes de usar un puerto: `ss -ltn`; después, confirmá
  que el contenedor está `Up` con los puertos publicados, no solo `Created`.
- **MinIO dev puede no levantar**: `dvu-minio-1` ocupa el 9000. Si pasa, el alta de
  avistamientos falla con "No se pudo consultar object storage".
- **Postgres tarda en estar listo de verdad**: `pg_isready` responde OK contra el servidor
  temporal de la fase de init y las migraciones fallan con "the database system is shutting
  down". Esperá con `docker exec ... psql -c 'SELECT 1'`.
- `psql`: el usuario es `$POSTGRES_USER` (`dev_user`), no `chiloe_user`.
- La tabla se llama `avistamiento_identificaciones`, no `identificaciones`.
- Datos de prueba dev: avistamiento `id=1` ("Arbol grande", `creado_por=2`, `aprobado`) y el
  usuario `pruebag@chiloe.dev` (id 2, password `PruebaG2026!`). **Credencial solo de desarrollo.**
- `especies-api` **no compila en este host** (faltan Pistache, libpqxx, json-schema-validator):
  `docker build --target tester -t especies-api-test services/especies-api`.

## Build del móvil

- `JAVA_HOME=/usr/lib/jvm/java-17-openjdk` — el JDK 26 del sistema rompe el plugin de Kotlin.
- Metro en el puerto **8082**: `chiloe-auth-dev` ocupa el 8081 en el host. (En release no hace
  falta Metro.)
- **No corras `npx prettier` en el repo móvil**: no hay `.prettierrc`, usa comillas dobles por
  defecto y reformatea archivos enteros fuera del estilo del proyecto. El formato se valida con
  `npm run lint`, que arrastra ~25 warnings `no-void` preexistentes: lo que importa es **0
  errores**.

## Detalle que conviene tener presente

El feed **no se cachea en SQLite**, a diferencia de la biblioteca. Es una desviación
deliberada de la regla offline-first: `foto_url` caduca a los ~15 minutos, así que un feed
guardado se vería sin imágenes, que es justo lo que se viene a mirar. Sin red muestra un error
con reintento. Las mutaciones (identificar, retirar) sí se encolan como siempre.

## Reglas que no se negocian

- Rama por cambio → commits → push → PR contra `master` → checks verdes → **el merge lo hago yo**.
- Nada de push a `master`, `--force`, `reset --hard` ni `--no-verify`.
- Migraciones numeradas y **nunca** editadas tras mergear.
- No commitear `.idea/editor.xml`, `.idea/vcs.xml`, el directorio sin trackear `diseño/`,
  keystores ni contraseñas de firma.
- Este archivo **sí** está trackeado: actualizalo al cerrar la sesión, en su propio PR.
- Avisame explícitamente cada desviación, default elegido u omisión. En español, directo.
