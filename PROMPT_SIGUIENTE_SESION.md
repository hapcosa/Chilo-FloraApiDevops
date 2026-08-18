# Prompt para la siguiente sesión

Copiá todo lo que sigue como primer mensaje de la sesión nueva.
Estado al 2026-08-18.

Hay otros dos prompts en la raíz para trabajos específicos, y **cada sesión va
en su propio `git worktree`**: [PROMPT_FASE_9.md](PROMPT_FASE_9.md) para
ejecutar el plan de la Fase 9, y
[PROMPT_CAMARA_SESION.md](PROMPT_CAMARA_SESION.md) para la cámara.

---

Seguimos con el sistema de biodiversidad de Chiloé:
`/home/obrero/programacion/Chilo-FloraApiDevops` (backend) y su submódulo
`mobile/`, que es su propio repo (`hapcosa/chiloe-biodiversidad-mobile`).

## Reglas innegociables

- Rama por cambio → commits → push → PR contra `master` → checks verdes →
  **el merge lo hago yo**. Nada de push a `master`, `--force`, `reset --hard`
  ni `--no-verify`.
- Migraciones numeradas y **nunca** editadas tras mergear. Las tablas no se
  crean desde código de aplicación.
- No commitear `.idea/editor.xml`, `.idea/vcs.xml`, el directorio sin trackear
  `diseño/`, keystores ni contraseñas de firma.
- Fotos: nunca multipart a la API. Presigned URL → subida directa → se notifica
  la key. EXIF sensible (GPS, serial) se borra salvo opt-in.
- Fungi: la comestibilidad siempre con el disclaimer de consultar a un experto.
- Sin dependencias nuevas sin justificarlas en el PR.
- Avisame explícitamente cada desviación, default elegido u omisión.
  En español, directo.
- Las claves SSH están en `~/.env`, se usan con `sshpass -e` y **nunca** se
  imprimen.
- Si te doy permiso para un update en producción, no borres los datos al
  terminar.
- Este archivo está trackeado: actualizalo al cerrar la sesión, en su propio PR.

## Lo primero: producción está atrasada

`master` avanzó varios PRs de backend y **nada de eso está desplegado**.
Mergear no despliega.

⚠️ **Antes de desplegar, mergear el PR #69.** El PR #67 (parques y áreas
protegidas) se mergeó por error contra `feat/mapa-endpoint-agregado` —una rama
que ya estaba mergeada—, así que ese trabajo **nunca llegó a `master`**: ni la
migración `0012`, ni el módulo `area_protegida`, ni las rutas nuevas del
`nginx.prod.conf`. El PR #69 lleva el mismo commit `5b06fab` a `master` y tiene
los checks en verde. Sin ese merge, el `git pull` de abajo no trae nada de
áreas protegidas y el gateway se reconstruye con el nginx viejo.

- **Prod está en la migración `0009`** (según el traspaso anterior; **verificalo
  antes de tocar nada**, no se pudo confirmar contra la BD). Faltan aplicar la
  `0010` (precisión declarada de avistamientos), la `0011` (índice espacial) y
  la `0012` (áreas protegidas, con sus datos sembrados).
- El `nginx.prod.conf` cambió (rutas nuevas de áreas protegidas), así que el
  **gateway** también hay que reconstruirlo — y ahí va además el arreglo del
  aviso del panel de curaduría, que está mergeado sin desplegar.

Orden: mergear el #69, después migraciones, después la API, después el gateway.

```bash
# en donaldchavez@10.244.117.161
cd ~/servicios/chiloe-biodiversidad-api && git pull --ff-only origin master
cd infrastructure/docker
C="docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env -f docker-compose.prod.yml"
$C build especies-api gateway
$C up especies-api-migrate      # sin -d, para ver el resultado
$C up -d especies-api gateway
```

⚠️ **Nombrá siempre el servicio concreto.** Un `build`/`up` pelado levantaría
un segundo `cloudflared` del mismo túnel, y Cloudflare repartiría el tráfico
entre las dos máquinas.

Verificación después: `curl -s https://api.budaicapital.com/api/v1/areas-protegidas | head`
y que el panel siga respondiendo 200 en `/curaduria/`.

## Dónde va la Fase 9

El plan es [docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md](docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md).
Van 8 de 13.

| PR | Qué | Estado |
|---|---|---|
| 1 | Quitar emojis de las fichas | ✅ mobile #32 |
| 2 | **Crear encuentro al terminar la captura** | ❌ **sin empezar** |
| 3 | Contar encuentros en vez de fichas abiertas | ✅ mobile #33 |
| 4 | Advertencia de fauna | ✅ mobile #34 |
| 5 | Bio, profesión y visibilidad en el perfil | ✅ backend #64 |
| 6 | Editar perfil de verdad | ✅ mobile #35 |
| 7 | Encuentros anteriores a la app | ✅ backend #65 + mobile #36 |
| 8 | **Mapa satelital de encuentros** | ❌ **sin empezar** |
| 9 | Endpoint agregado para el mapa | ✅ backend #66 |
| 10 | Parques y áreas protegidas | ⚠️ backend #69 (el #67 no llegó a `master`) |
| 11 | **Insignias** | ❌ sin empezar |
| 12 | **Pantalla de usuarios del panel** | ❌ sin empezar |
| 13 | **Postular a curar** | ❌ sin empezar |

El siguiente natural es el **PR 8**: el backend del mapa ya está entero
(endpoint agregado + áreas protegidas) y sin la pantalla móvil no se ve nada de
eso. Después el **PR 2**, que es el que cierra el flujo de la cámara.

Del PR 12 hay una **decisión pendiente conmigo, no la tomes solo**: el listado
de usuarios cruza dos servicios (`users` vive en `auth-service`, las
asignaciones a categorías en `especies-api`), así que hay que elegir si el panel
consulta a los dos o si uno expone la vista combinada.

## Cosas que ya están y conviene no redescubrir

- El panel de curaduría está en producción en
  `https://api.budaicapital.com/curaduria/` y **tiene login con Google**. Mi
  cuenta `hapcosa@gmail.com` (id 4) es `admin`.
- **Nadie puede postular a curar todavía**: el endpoint
  `POST /api/v1/postulaciones` y la bandeja de revisión existen desde la
  migración `0005`, pero ningún cliente tiene interfaz para postular. Es el
  PR 13.
- **Un admin tampoco puede repartir permisos por la web**: los endpoints
  `POST`/`DELETE /api/v1/categorias/:id/moderadores/:usuarioId` existen y son
  admin-only, pero ninguna pantalla los llama. Es el PR 12.
- La **key de Google Maps** va en el `AndroidManifest.xml` del APK (Maps SDK for
  Android), **no** en el env del backend, y **restringida por nombre de paquete
  y huella SHA-1**. Una key sin restringir es facturable por cualquiera que la
  saque del APK. La `GOOGLE_MAP_API` del `.env` local no la usa nadie.

## Entorno

- Build de Android: `JAVA_HOME=/usr/lib/jvm/java-17-openjdk` (el default del
  host es Java 26 y el plugin Gradle de RN no parsea esa versión).
- `especies-api` **solo compila dentro de Docker**: al host le faltan Pistache y
  libpqxx.
- `applicationId cl.chiloe.biodiversidad`. El `release` está firmado con la
  clave de debug y `enableProguardInReleaseBuilds = false`.
- `mobile/src/config/appConfig.ts`: debug → `http://localhost:8080`,
  release → `https://api.budaicapital.com`. Solo el APK release habla con prod.
- Dispositivo por adb en red (reconfirmá la IP con `adb devices`).
  Logs: `adb logcat -d --pid=$(adb shell pidof cl.chiloe.biodiversidad)`.
- Producción es `donaldchavez@10.244.117.161`, checkout en
  `~/servicios/chiloe-biodiversidad-api`, proyecto compose `chiloe-prod`, env
  **fuera del repo** en `~/.config/chiloe-prod/chiloe.env`. El host viejo
  `10.244.19.205` es el entorno de test. Postgres: usuario `chiloe_prod`, base
  `chiloe_biodiversidad`.
- Los datos viven en volúmenes Docker con nombre (`chiloe-prod_postgres_data`,
  `_minio_data`, `_redis_data`): migrarlos requiere `pg_dump` y un `tar` del
  volumen, no copiar un directorio.

## Otros pendientes, más viejos

- **Prueba manual de la cámara en el teléfono**: nunca se hizo. Es lo primero
  de [PROMPT_CAMARA_SESION.md](PROMPT_CAMARA_SESION.md).
- Fotos para una especie con "hartas fotos": bloqueado esperándome. No bajes
  imágenes de licencia indeterminada a producción.
- Verificación del Paso 3, puntos que faltan: 3 (filtros y paginación),
  5 (visibilidad), 6 (captura de foto + PUT presigned), 9 (sync del cache de
  especies). El 7 solo cuando ponga modo avión a mano.
- Sin decidir: un PR aparte por la contraseña de Postgres en texto plano que
  `especies-api` imprime al arrancar y queda en los logs del contenedor de
  producción.
