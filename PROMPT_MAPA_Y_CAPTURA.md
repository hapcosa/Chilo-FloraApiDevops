# Prompt para la sesión siguiente

Copiá todo lo que sigue como primer mensaje de la sesión nueva.
Estado al 2026-08-26, con la Fase 9.4 cerrada, mergeada **y desplegada**.

> Para la sesión de **diseño visual de la app** no uses este archivo: está
> [PROMPT_DISENO_APP.md](PROMPT_DISENO_APP.md), que es su propio encargo.

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
  `diseño/`, keystores, contraseñas de firma **ni `android/local.properties`**.
- Fotos: nunca multipart a la API. Presigned URL → subida directa → se notifica
  la key. EXIF sensible (GPS, serial) se borra salvo opt-in.
- Fungi: la comestibilidad siempre con el disclaimer de consultar a un experto.
- Sin dependencias nuevas sin justificarlas en el PR.
- Avisame explícitamente cada desviación, default elegido u omisión.
  En español, directo.
- Las claves SSH están en `~/.env`, se usan con `sshpass -e` y **nunca** se
  imprimen. **El acceso SSH a producción está bloqueado por el clasificador de
  permisos de Claude Code**: aunque el host es alcanzable por ZeroTier y el
  puerto 22 responde, tanto `ssh` como leer `~/.env` se deniegan. Pedime a mí
  los comandos que haya que correr allá, o corrélos con el prefijo `!` para que
  la salida caiga en la conversación.
- Este archivo está trackeado: actualizalo al cerrar la sesión, en su propio PR.

---

## Estado: producción está al día

Desplegado y **verificado** el 2026-08-26: PRs 77, 79, 80 y 82. Migraciones
hasta la `0013`, 103 especies clasificadas en veinte subgrupos, `sin_subgrupo`
en 0, 7 áreas protegidas, 5 reinos.

Mergeado en mobile: **#40** (íconos de trazo, etiquetas sin truncar), **#41**
(filtro por subgrupo en la biblioteca), **#42** (arreglo del cuelgue al
actualizar la app).

**No hay deploy automático.** El workflow que existía apuntaba a EKS y fallaba
siempre; se borró en el PR 75. Redesplegar es a mano, con el bloque de comandos
del final de este archivo.

## Lo que quedó sin verificar

**La fila de chips de subgrupo y la portada con contenido no se vieron nunca en
pantalla.** La app pidió login al arrancar (la sesión se perdió durante los
arranques fallidos del bug del #42) y no tengo las credenciales. Es lo primero
que conviene cerrar: entrá en la app y pedime que verifique.

---

## Lo que sigue, en el orden que yo haría

### 1. `queryStr` no decodifica nada

El arreglo del PR 73 fue solo para `bbox`. Cualquier búsqueda con espacios o
acentos (`?q=zorro%20chilote`) llega mal a los demás endpoints. Es un PR chico y
aparte. **Ahora importa más**: la biblioteca cruza búsqueda de texto con el
filtro de subgrupo.

### 2. El mapa

Dos problemas distintos, los dos reales:

- **Nunca lee el GPS.** No es que apunte mal: `MapaScreen.tsx` monta el
  `MapView` con `initialRegion={REGION_CHILOE}` y nada más. No hay
  `showsUserLocation`, ni botón de "mi ubicación", ni se pide el permiso. Hay
  que implementarlo, incluyendo el permiso en runtime y el caso de que lo
  nieguen.
- **Se mueve con tirones.** Cada gesto dispara `onRegionChangeComplete` →
  `setRegion` → re-render de todos los `Marker` y `Circle`, más una llamada de
  red por movimiento. `regionesEquivalentes` amortigua las llamadas pero no el
  re-render. Memoizar los marcadores y sacar `region` del estado que redibuja
  es por donde empezaría.

Anotado de paso: el `title` del `Marker` —donde se arregló el "1 encuentros"—
**es inalcanzable en la práctica**. Al tocar el pin, el `onPress` navega a la
ficha de la especie antes de que el callout se dibuje. Si el tap debería mostrar
el resumen de la celda en vez de saltar a la ficha, es una decisión mía: preguntá.

### 3. Fase 9 — PR 13: postular a curador desde la app

**Está a medias, y la mitad que falta es la que ve el usuario.** El backend está
completo (migración `0005`, controller, service, ruta en el gateway) y el panel
de curaduría ya tiene su pantalla para aprobar o rechazar. En `mobile/src` no
hay una sola línea: hoy se pueden aprobar postulaciones que nadie puede enviar.

### 4. Fase 9 — PR 12: pantalla de usuarios del panel

**Bloqueado por una decisión mía**: `users` vive en `auth-service` y las
asignaciones a categorías en `especies-api`. Hay que elegir si el panel consulta
a los dos o si uno expone la vista combinada. Preguntame antes de escribir código.

### 5. Fase 9 — PR 11: insignias

Sin empezar.

### 6. Diseño de la app

Su propio encargo en [PROMPT_DISENO_APP.md](PROMPT_DISENO_APP.md).

### 7. Fotos reales

Bloqueado esperándome. No bajes imágenes de licencia indeterminada a producción.

### 8. Sin decidir

La contraseña de Postgres en texto plano que `especies-api` imprime al arrancar
y queda en los logs del contenedor de producción.

### Verificación del Paso 3

Faltan los puntos 3 (filtros y paginación), 5 (visibilidad) y 9 (sync del cache
de especies). El 7 solo cuando ponga modo avión a mano.

---

## Lo que se aprendió y no conviene volver a aprender

- **Las migraciones corren antes que los seeds.** Un backfill dentro de una
  migración no alcanza a las fichas que un entorno nuevo siembra después. Por
  eso el mapeo familia→subgrupo vive en la tabla `familia_subgrupo` y el seed
  `0003` lo reaplica.
- **El seed `0003` es parte del despliegue, no un opcional.** La `0013`
  clasifica por familia y deja fuera lo que ninguna familia mapea —monera no
  tiene subgrupos a propósito—. En producción quedaron 2 fichas de monera en
  `NULL` hasta correrlo.
- **Un índice sobre una columna añadida por `ALTER` no puede vivir junto a las
  tablas.** `CREATE TABLE IF NOT EXISTS` no toca la tabla que ya existe en el
  teléfono, así que el índice explota con `no such column`, rompe
  `initializeDatabase` entera y la app se cuelga en "Cargando sesión...".
  **Solo se ve actualizando sobre una instalación previa**: ni CI ni una
  instalación limpia lo detectan. El orden es tablas → ALTER → índices, y hay un
  test que lo defiende.
- **Verificar en el teléfono no es un trámite.** Ese bug y la truncación de
  "Guardad…" salieron los dos de mirar la pantalla, con los tests en verde.
- **Se puede probar una migración sin levantar el compose**: un
  `docker run -d -e POSTGRES_PASSWORD=postgres -e POSTGRES_DB=chiloe_flora -p 55432:5432 postgres:16`
  y después `DB_HOST=localhost DB_PORT=55432 DB_USER=postgres DB_PASSWORD=postgres`
  con `./scripts/migrate.sh` y `./scripts/seed.sh`.
- **Al mergear mobile hay que subir el puntero del submódulo** en el repo
  backend, en su propio PR. Es fácil de olvidar.

---

## Entorno

- Build de Android: `JAVA_HOME=/usr/lib/jvm/java-17-openjdk`. El default del
  host es **Java 26** y el plugin Gradle de RN revienta con
  `IllegalArgumentException: 26.0.2`. No toques `gradle.properties` por eso: es
  del entorno, no del repo. La release incremental tarda ~4 min; desde limpio, 12.
- Tiene que ser **release**: `src/config/appConfig.ts` manda el debug a
  `http://localhost:8080` y solo el release habla con `api.budaicapital.com`.
  Si la pantalla sale roja con "Unable to load script", instalaste el debug.
- El teléfono se conecta por wifi. **Es un teléfono en uso**: confirmá el foco
  con `adb shell dumpsys window | grep mCurrentFocus` antes de tocar nada. Ya
  pasó abrir la galería del dueño con tres fotos seleccionadas. Y `adb exec-out
  screencap` escribe en el cwd, que se resetea después de un comando en
  background: usá rutas absolutas o vas a ensuciar el repo.
- `especies-api` **solo compila dentro de Docker**: al host le faltan Pistache y
  libpqxx. Ignorá los errores de clang sobre esos headers. Los tests de gtest se
  reproducen con `docker build --target tester services/especies-api`; para un
  subconjunto:
  `docker run --rm --entrypoint /app/build/tests/unit_tests <img> --gtest_filter='*LoQueSea*'`.
- `applicationId cl.chiloe.biodiversidad`. El `release` está firmado con la
  clave de debug y `enableProguardInReleaseBuilds = false`.
- La key de Google Maps sale de `GOOGLE_MAP_API` en el `.env` y va a
  `mobile/android/local.properties` como `MAPS_API_KEY`. Ese archivo **no se
  commitea**.
- `npm run lint` en mobile arrastra **48 warnings `no-void`** preexistentes.
  0 errores es el criterio; no salgas a limpiarlos sin PR propio.
- El CI del backend corre **solo en `pull_request`** desde el PR #70. El job
  `test-especies-api` se cuelga a veces en `Install postgresql-client` y GitHub
  lo cancela a los 20 min: `gh run rerun <run-id> --failed`. La duración que
  muestra la UI **suma los dos intentos**.
- Si agregás un endpoint nuevo, agregá su `location` en `nginx.prod.conf` **y**
  en `nginx.dev.conf`: Nginx lista una por ruta y lo que no matchea cae en
  `location /`, devolviendo el índice del gateway con un **200**. Y ojo: casi
  todas las rutas de `/api/v1/` exigen sesión con `auth_request`, así que un
  `curl` sin token contesta **401 y eso es lo correcto** — no es un síntoma de
  nada.

---

## Producción

- `donaldchavez@10.244.117.161`, checkout en
  `~/servicios/chiloe-biodiversidad-api`, proyecto compose `chiloe-prod`, env
  **fuera del repo** en `~/.config/chiloe-prod/chiloe.env`. El host viejo
  `10.244.19.205` es el entorno de test. Postgres: usuario `chiloe_prod`, base
  `chiloe_biodiversidad`.
- La máquina **no tiene IP pública**: se llega por **ZeroTier**
  (`10.244.0.0/16`, interfaz `ztbpaiczc3`) y el túnel Cloudflare solo publica
  `api.budaicapital.com` y `storage.budaicapital.com`. Un runner de GitHub no la
  alcanza. Si alguna vez se automatiza el deploy, los caminos son SSH por
  Cloudflare Access con service token, unir el runner a ZeroTier, o un runner
  self-hosted. **Eso lo decido yo.**
- Al redesplegar, **nombrá siempre los servicios concretos**: un `build`/`up`
  pelado levantaría un segundo `cloudflared` del túnel de Chiloé y Cloudflare
  repartiría el tráfico entre dos máquinas. En esa máquina corren tres
  `cloudflared` de proyectos distintos.

```bash
# en donaldchavez@10.244.117.161
cd ~/servicios/chiloe-biodiversidad-api && git pull --ff-only origin master
cd infrastructure/docker
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml build especies-api gateway
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up especies-api-migrate      # sin -d
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up -d especies-api gateway
# los seeds no los aplica el migrate: son parte del despliegue
docker exec -i chiloe-postgres psql -U chiloe_prod -d chiloe_biodiversidad \
  < ~/servicios/chiloe-biodiversidad-api/services/especies-api/seeds/0003_subgrupos_por_familia.sql
```

`nginx.conf` es un **bind mount** (`docker-compose.prod.yml:251`), así que un
`up -d --build` no hace que el Nginx que ya corre relea el archivo: hace falta
`docker exec chiloe-gateway nginx -t && docker exec chiloe-gateway nginx -s reload`.
Un reinicio de la máquina también lo resuelve, porque recrea el contenedor —
así fue como el PR 79 terminó aplicándose solo.

⚠️ En **zsh** no sirve meter el comando en una variable y hacer `$C build ...`:
zsh no parte la variable en palabras y falla con "no such file or directory".
Usá el comando completo, o `${=C}`.

Los contenedores se llaman `chiloe-postgres`, `chiloe-gateway`,
`chiloe-especies-api`, `chiloe-auth`, `chiloe-minio`, `chiloe-redis`,
`chiloe-cloudflared` (tienen `container_name` explícito).

Para verificar la base sin depender de tokens:

```bash
docker exec chiloe-postgres psql -U chiloe_prod -d chiloe_biodiversidad -c \
  "SELECT count(*) AS sin_subgrupo FROM especies WHERE categoria_id IS NULL;"
```
