# Prompt para la sesión siguiente

Copiá todo lo que sigue como primer mensaje de la sesión nueva.
Estado al 2026-08-27. **Producción está desplegada, verificada y al día**: no
hay nada urgente que hacer al arrancar. La sesión nueva empieza por decidir,
no por desplegar.

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

## Estado: producción está al día, con las insignias

Desplegado y **verificado el 2026-08-27**: PR 92 (insignias) en producción,
migraciones hasta la **`0014`**, 11 insignias en el catálogo, y el recálculo
corrido una vez (**`{"otorgadas":1,"success":true}`**). Las rutas
`/api/v1/insignias*` responden 401 sin token, que es lo correcto.

Antes de eso, desplegado y verificado el 2026-08-26: PRs 77, 79, 80 y 82, 103
especies clasificadas en veinte subgrupos, `sin_subgrupo` en 0, 7 áreas
protegidas, 5 reinos.

Mergeado en mobile: **#40** (íconos de trazo), **#41** (filtro por subgrupo),
**#42** (cuelgue al actualizar), **#43** (ubicación en el mapa) con el **#89**
del backend, **#44** (postular a curador) con el **#91**, y **#45** (insignias
en los perfiles) con el **#94**. El puntero del submódulo apunta a `6431792`.

**No hay deploy automático.** El workflow que existía apuntaba a EKS y fallaba
siempre; se borró en el PR 75. Redesplegar es a mano, con el bloque de comandos
del final de este archivo.

**El APK con insignias está instalado en el teléfono** (2026-08-27 08:51,
release, sobre la instalación previa). La actualización sobre una instalación
existente **no** reprodujo el cuelgue del índice SQLite.

## Lo que quedó sin verificar

- **Las insignias en un perfil público ajeno.** El perfil propio sí se verificó
  (ver abajo), pero el público no se puede probar hoy: los únicos usuarios que
  aparecen en el feed son el **#6** y el **#7**, y ninguno publicó su perfil —
  la app muestra correctamente "Perfil no disponible". Para verificarlo hay que
  o publicar el perfil de alguno, o que mi cuenta gane una insignia, y las dos
  cosas escriben en la BD de producción. **Pedímelo y lo hacemos.**
- **A quién le tocó la única insignia otorgada** no se sabe. Vale un
  `SELECT ui.usuario_id, i.codigo FROM usuario_insignias ui JOIN insignias i ON
  i.id = ui.insignia_id;` en prod para distinguir "el criterio funciona y hay
  poca data" de "el criterio matchea algo raro".
- **La pantalla de postulación a curador se verificó** el 2026-08-27, pero
  **no mandé una postulación real**: escribiría en la BD de producción con tu
  cuenta y caería en la bandeja del panel. Pedímelo y la mando.

## Lo que sí se verificó en el teléfono el 2026-08-27

- **La sección Insignias del perfil propio funciona y habla con el servidor.**
  Con 0 encuentros muestra el estado vacío ("Todavía no tienes insignias") y
  debajo el bloque **POR GANAR** con las ocho automáticas y su criterio:
  primer encuentro (1 aprobado), observador, constante, curioso (10 especies
  distintas), coleccionista (30), tres reinos, cinco reinos, en comunidad
  (5 encuentros identificados por otros). Esos criterios solo pueden venir del
  catálogo de la API, así que la llamada a `/api/v1/insignias/mias` está bien.
- **El perfil público responde bien el caso "no publicado"**, con el texto que
  aclara que los encuentros compartidos siguen visibles.

## Deuda inmediata

Ninguna urgente. Lo único abierto es una decisión tuya (el PR 12, punto 1 de
la lista de abajo).

---

## Lo que sigue, en el orden que yo haría

### 1. Fase 9 — PR 12: pantalla de usuarios del panel

**Bloqueado por una decisión tuya**: `users` vive en `auth-service` y las
asignaciones a categorías en `especies-api`. Hay que elegir si el panel consulta
a los dos o si uno expone la vista combinada. Preguntame antes de escribir
código. Acá encaja también el **botón de recálculo de insignias** en el panel de
curaduría, que hoy no existe: el endpoint se llama a mano.

### 2. "3 reinos" quiere decir dos cosas distintas en la misma pantalla

Encontrado el 2026-08-27 mirando el perfil. La tarjeta de estadísticas dice
**"3 reinos"** mientras la insignia **"Tres reinos — 3 reinos distintos"**
sigue en gris, tres centímetros más abajo. No es un bug: `PerfilScreen.tsx:122`
cuenta los reinos de las **fichas consultadas** (SQLite local), y la insignia
cuenta los reinos con **encuentros aprobados** (servidor). Son dos métricas con
la misma palabra, y puestas juntas se leen como una contradicción.

Arreglo barato: renombrar la tarjeta a algo como "reinos explorados" o
"reinos vistos". **Decisión tuya**, porque toca copy visible.

### 3. El mapa — hecho a medias (PR 43, mergeado)

- **El GPS ya se lee.** Botón de "mi ubicación", permiso en runtime, punto azul
  y mensajes distintos para "no me dejaron" y "el GPS no respondió".
- **Los tirones siguen sin diagnosticar.** Memoicé los overlays y saqué
  `region` del estado, pero al medir con `dumpsys gfxinfo` **no hubo mejora**:
  master 13/659 frames con jank (1,97%), la rama 10/668 (1,50%), p95 idéntico.
  Es ruido, porque hoy producción pinta **una** celda y 7 áreas. La causa real
  está sin encontrar; el cambio se justifica solo por cuando haya densidad.
- El `onPress` del `Marker` **no** navega a la ficha: llama a `filtrarPorCelda`.
  El callout se pierde igual, porque `setEspecieId` recarga las celdas y los
  `Marker` se desmontan. **Sigue siendo una decisión tuya** qué debería hacer el
  tap: filtrar, mostrar el resumen de la celda, o distinguir tap de long-press.

### 4. Insignias: lo que falta

- **No se muestran en el feed**, aunque el plan lo pide. El feed **no muestra
  autores**: las tarjetas llevan especie, foto y grado, nunca quién lo registró.
  No hay nombre al lado del cual ponerlas. El único lugar con personas es la
  lista de identificaciones del detalle ("Usuario #N"), y ahí harían falta N
  peticiones por pantalla. Antes de eso conviene un
  `GET /api/v1/insignias?usuarios=1,2,3`. `InsigniasRow` ya está lista para
  colgarse donde aparezca la autoría.
- **No hay aviso al postulante a curador** cuando le resuelven: la app dice
  "te avisaremos" y hoy eso es entrar y mirar.

### 5. Diseño de la app

Su propio encargo en [PROMPT_DISENO_APP.md](PROMPT_DISENO_APP.md).

### 6. Fotos reales

Bloqueado esperándome. No bajes imágenes de licencia indeterminada a producción.

### 7. Sin decidir

La contraseña de Postgres en texto plano que `especies-api` imprime al arrancar
y queda en los logs del contenedor de producción.

### Verificación del Paso 3

Faltan los puntos 3 (filtros y paginación), 5 (visibilidad) y 9 (sync del cache
de especies). El 7 solo cuando ponga modo avión a mano.

---

## Lo que se aprendió y no conviene volver a aprender

- **El `nginx.conf` del gateway es un bind mount de *archivo*, y por eso un
  `git pull` no lo actualiza nunca.** Diagnosticado el 2026-08-27, después de
  que el deploy de las insignias dejara la ruta nueva sin funcionar. Docker
  resuelve ese mount **por inode al crear el contenedor**; `git pull` escribe un
  archivo nuevo y lo renombra encima, así que el inode cambia y el contenedor
  sigue leyendo la copia vieja indefinidamente. Los síntomas engañan:
  `docker exec chiloe-gateway grep -c insignias /etc/nginx/nginx.conf` da **0**
  aunque el archivo del host sí la tenga, `nginx -t` **pasa** (la config vieja
  es válida) y `nginx -s reload` no cambia nada. `docker inspect` muestra el
  mount apuntando al archivo correcto, que es lo que despista. La cura es
  **`up -d --force-recreate gateway`**, nombrando el servicio. El contenedor
  llevaba creado desde el **2026-08-18** arrastrando esa config en todos los
  deploys posteriores. Esto explica lo que este archivo anotaba como misterio:
  el PR 79 no "se aplicó solo", lo aplicó el reinicio de la máquina al recrear
  el contenedor.
- **Una ruta de `/api/v1/` que falta en el Nginx que corre devuelve 200, no
  404**: cae en `location /` y contesta el índice estático del gateway. La
  forma barata de detectarlo desde afuera es comparar contra una ruta que sí
  existe — `/api/v1/especies` da 401 — y mirar el `last-modified` de la
  respuesta.
- **El recálculo de insignias se puede disparar sin token de admin**, desde
  dentro de la red de Docker: la API confía en `X-User-Id`/`X-User-Role` que
  inyecta el gateway tras el `auth_request`
  (`services/especies-api/src/utils/request_identity.cpp:4`), y el contenedor
  de `especies-api` ya trae `curl`. Sirve cuando la cuenta admin entra por
  Google Sign-In y no hay contraseña que mandar por `curl`:
  `docker exec chiloe-especies-api curl -s -X POST -H 'X-User-Id: 1'
  -H 'X-User-Role: admin' http://localhost:9080/api/v1/insignias/recalcular`.
  El `X-User-Id` es arbitrario: `recalcular()` no lo usa. **La contracara es que
  la frontera de autorización es el gateway**: cualquiera con acceso a esa red
  puede hacerse pasar por admin. Es el diseño ya decidido, no un bug del PR 92.
- **El recálculo de insignias se probó contra un Postgres real**, no solo con
  gtest: `docker run postgres:16-alpine`, las migraciones en orden y datos
  sembrados. Ahí se confirmó que la autoidentificación no suma y que correrlo
  dos veces otorga cero. Los tests unitarios no tocan SQL.
- **Las tablas del catálogo no traen seed de especies**: una BD recién migrada
  tiene `especies` vacía, así que cualquier prueba de agregación hay que
  sembrarla a mano (ojo: `generos` no tiene columna `reino`, la tiene
  `familias`).
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
  test que lo defiende. Por eso el APK se instala con `adb install -r` sobre la
  versión anterior, nunca desinstalando primero.
- **Verificar en el teléfono no es un trámite.** Ese bug, la truncación de
  "Guardad…" y lo de los dos significados de "reinos" salieron los tres de
  mirar la pantalla, con los tests en verde.
- **Se puede probar una migración sin levantar el compose**: un
  `docker run -d -e POSTGRES_PASSWORD=postgres -e POSTGRES_DB=chiloe_flora -p 55432:5432 postgres:16`
  y después `DB_HOST=localhost DB_PORT=55432 DB_USER=postgres DB_PASSWORD=postgres`
  con `./scripts/migrate.sh` y `./scripts/seed.sh`.
- **Las dos APIs no reportan los errores igual.** El `auth-service` (Go) manda
  `message` y la `especies-api` (C++) manda `error`. `apiClient` solo miraba el
  primero, así que **todo** error de la API de especies llegaba a la UI como un
  `HTTP 400` pelado. Arreglado en el PR 44; si aparece un servicio nuevo,
  revisá con cuál de los dos formatos habla.
- **Medir antes de afirmar que algo mejoró el rendimiento.** La memoización del
  mapa parecía obvia y no movió la aguja: `adb shell dumpsys gfxinfo <pkg>` con
  la misma secuencia de gestos en las dos builds es la forma barata de saberlo.
- **Al mergear mobile hay que subir el puntero del submódulo** en el repo
  backend, en su propio PR. Es fácil de olvidar.
- **En el checkout de producción, ` M mobile` es ruido**, no trabajo sin
  commitear: es el puntero del submódulo, que allá nunca se inicializa.

---

## Entorno

- Build de Android: `JAVA_HOME=/usr/lib/jvm/java-17-openjdk`. El default del
  host es **Java 26** y el plugin Gradle de RN revienta con
  `IllegalArgumentException: 26.0.2.1`. No toques `gradle.properties` por eso:
  es del entorno, no del repo. La release incremental tarda ~1 min si solo
  cambió JS; desde limpio, 12.
- Tiene que ser **release**: `src/config/appConfig.ts` manda el debug a
  `http://localhost:8080` y solo el release habla con `api.budaicapital.com`.
  Si la pantalla sale roja con "Unable to load script", instalaste el debug.
- El teléfono se conecta por wifi. **Es un teléfono en uso**: confirmá el foco
  con `adb shell dumpsys window | grep mCurrentFocus` antes de tocar nada, y
  **volvé a confirmarlo antes de cada captura** — el 2026-08-27 el dueño volvió
  a su app entre el `dumpsys` y el `screencap`, y la captura salió con una
  pantalla suya. Ya pasó también abrir la galería con tres fotos seleccionadas.
  Y `adb exec-out screencap` escribe en el cwd, que se resetea después de un
  comando en background: usá rutas absolutas o vas a ensuciar el repo.
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
- `npm run lint` en mobile arrastra **51 warnings `no-void`** preexistentes.
  0 errores es el criterio; no salgas a limpiarlos sin PR propio.
- El CI del backend corre **solo en `pull_request`** desde el PR #70. El job
  `test-especies-api` se cuelga a veces en `Install postgresql-client` y GitHub
  lo cancela a los 20 min: `gh run rerun <run-id> --failed`. La duración que
  muestra la UI **suma los dos intentos**.
- Si agregás un endpoint nuevo, agregá su `location` en `nginx.prod.conf` **y**
  en `nginx.dev.conf`, y acordate del `--force-recreate` del gateway al
  desplegar (ver arriba). Ojo: casi todas las rutas de `/api/v1/` exigen sesión
  con `auth_request`, así que un `curl` sin token contesta **401 y eso es lo
  correcto** — no es un síntoma de nada.
- **Producción no tiene `less`**: `git log` allá falla con "unable to execute
  pager 'less'". Usá `git --no-pager log`.

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
  -f docker-compose.prod.yml up -d especies-api
# el gateway SIEMPRE con --force-recreate si cambió nginx.prod.conf:
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up -d --force-recreate gateway
# los seeds no los aplica el migrate: son parte del despliegue
docker exec -i chiloe-postgres psql -U chiloe_prod -d chiloe_biodiversidad \
  < ~/servicios/chiloe-biodiversidad-api/services/especies-api/seeds/0003_subgrupos_por_familia.sql
```

Después de desplegar, verificá **desde afuera** que las rutas nuevas no caigan
en `location /`:

```bash
for p in /api/v1/insignias /api/v1/especies /health; do
  printf "%-26s %s\n" "$p" "$(curl -s -o /dev/null -w '%{http_code}' https://api.budaicapital.com$p)"
done
# lo correcto: 401, 401, 200. Un 200 en una ruta de /api/v1/ es el síntoma
# de que el gateway no releyó nginx.prod.conf.
```

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
docker exec chiloe-postgres psql -U chiloe_prod -d chiloe_biodiversidad -c \
  "SELECT max(version) FROM schema_migrations; SELECT count(*) FROM insignias;"
```
