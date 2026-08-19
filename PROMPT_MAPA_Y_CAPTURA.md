# Prompt para la sesión de portada, filtros e íconos

Copiá todo lo que sigue como primer mensaje de la sesión nueva.
Estado al 2026-08-19, con cuatro PRs abiertos esperando merge.

---

Seguimos con el sistema de biodiversidad de Chiloé:
`/home/obrero/programacion/Chilo-FloraApiDevops` (backend) y su submódulo
`mobile/`, que es su propio repo (`hapcosa/chiloe-biodiversidad-mobile`).

La sesión anterior cerró la captura y arregló el mapa del lado del servidor.
Falta **verificar el mapa en el teléfono contra producción** —lo bloquea un
merge y un redespliegue que hago yo— y arrancar la **Fase 9.4**: portada viva,
filtros por subgrupo e íconos de navegación.

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
  imprimen.
- Este archivo está trackeado: actualizalo al cerrar la sesión, en su propio PR.

---

## Lo que cerró la sesión anterior

- **La app no arrancaba**: no era el mapa ni un crash nativo. El APK instalado
  era el **debug**, que no lleva bundle JS y exige Metro en `localhost:8081`.
  Con `assembleRelease` arrancó. No hubo cambio de código.
- **El mapa se ve**: teselas satelitales sobre Chiloé con la key de
  `GOOGLE_MAP_API` del `.env` (el prompt anterior decía que esa key "no la usa
  nadie" — era falso, es la buena). Los pines de áreas protegidas caen bien, o
  sea el `bbox` no está invertido.
- **Los círculos de la comunidad fallaban con 400 en 4 ms**: `parseBbox` del
  backend partía por comas un valor que llegaba como `%2C`. Pistache **no**
  decodifica los valores de query. Arreglado con `utils::percentDecode` y 7
  tests → **backend PR 73**.
- **Captura → encuentro** (PR 2 de la Fase 9) → **mobile PR 38**.
- **El deploy automático nunca existió**: el workflow apuntaba a EKS/ECR y
  fallaba en cada push a master hace meses. Borrado → **backend PR 75**.

## PRs abiertos, esperando que yo los merge

| PR | Repo | Qué | Checks |
|---|---|---|---|
| 73 | backend | `percentDecode` del bbox — **desbloquea el mapa** | verde |
| 74 | backend | Plan de la Fase 9.4 | verde |
| 75 | backend | Borrar el deploy a EKS | recién abierto |
| 38 | mobile | Encuentro tras la captura | verde |

**Empezá preguntándome si ya los mergeé.** El trabajo 0 depende del 73.

---

## Trabajo 0 — Terminar de verificar el mapa

Requiere que yo haya mergeado el **PR 73** y redesplegado `especies-api` en
producción. Pedímelo; el redespliegue lo hago yo o te paso el resultado.

Después, con el release instalado en el teléfono, mirá en la pestaña **Mapa**:

- Que los **círculos de la comunidad** aparezcan y caigan sobre Chiloé, no en
  el Golfo de Guinea. Es lo que arregla el PR 73.
- Que el **filtro por reino** recargue las celdas.
- Que tocar un **punto caliente** filtre por esa especie.
- Que el chip alterne **Satelital ↔ Híbrida**.
- Que las tres capas se prendan y apaguen por separado.
- Que aparezcan **7 áreas protegidas**: PN Chiloé, Tantauco, Tepuhueico,
  Ahuenco, Islotes de Puñihuil, Humedal de Caulín, Humedales de Putemún.

Y probá el flujo nuevo de **Capturar** (PR 38, si está mergeado): disparo →
revisión → Crear encuentro / Repetir / Descartar, incluyendo
"todavía no sé cuál es". Nunca se probó en el teléfono, solo en CI.

### Decisión pendiente conmigo

El mapa quedó como **séptima pestaña** y con siete las etiquetas se truncan
("Comun…", "Guarda…"). La alternativa es colgarlo del stack de Comunidad.
**No la tomes solo**: decidila conmigo mirando el teléfono. Se solapa con el
PR 16 de abajo, que rehace la barra igual.

Comandos:

```bash
cd mobile/android
JAVA_HOME=/usr/lib/jvm/java-17-openjdk ./gradlew :app:assembleRelease
adb devices        # reconfirmá la IP, cambia sola
adb install -r app/build/outputs/apk/release/app-release.apk
adb logcat -d --pid=$(adb shell pidof cl.chiloe.biodiversidad)
```

Tiene que ser **release**: `src/config/appConfig.ts` manda el debug a
`http://localhost:8080` y solo el release habla con `api.budaicapital.com`.
Si la pantalla sale roja con "Unable to load script", instalaste el debug.

---

## Trabajo 1 — Fase 9.4, "que la app se vea viva"

El plan está en
[docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md](docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md),
sección **Fase 9.4** (la agrega el PR 74). Son tres PRs, en este orden:

### PR 14 — Portada viva *(backend + mobile)*

Hoy la portada muestra **una especie arbitraria por reino** y no cambia nunca.
Tiene que mostrar **últimas especies publicadas**, **últimas ediciones** y
**últimos encuentros de la comunidad**.

Un solo endpoint `GET /api/v1/portada` que devuelva las tres listas, no tres
llamadas desde el teléfono.

⚠️ **No filtres esto en el cliente**: los encuentros de la portada tienen que
respetar lo mismo que el mapa —privados fuera, y nada que revele la ubicación
exacta de una especie amenazada (ADR #23)—. Si la portada muestra coordenadas,
tira abajo la ofuscación del mapa.

### PR 15 — Filtros por subgrupo *(backend + mobile)*

Dentro de `animalia`: aves, peces, reptiles, mamíferos, anfibios. Y lo que
corresponda en los otros reinos.

**El eje ya existe**: la migración `0004` creó `categorias_moderacion`
—"subgrupo curable dentro de un reino, sin jerarquía"— y `especies.categoria_id`.
Está backfilleada con cinco categorías "general", una por reino. La
recomendación del plan es **reusarla** en vez de crear una tabla `clases` nueva.

El costo de reusarla, que hay que asumir a conciencia: acopla el eje de
**permisos de curaduría** con el eje de **navegación**. Crear la categoría
"Aves" para que la app filtre significa crear también una unidad de moderación
que alguien puede curar. Si eso no te cierra, decidilo conmigo antes de
escribir código.

Las subcategorías las crea un admin por la API; hay que sembrarlas.

### PR 16 — Íconos en vez de emojis *(mobile)*

Los íconos de la barra son emojis del sistema (`🏠 🔎 📷 🗺️ 👥 🔖 🙋`) puestos
como `<Text>` en `src/navigation/AppNavigator.tsx`. Se ven como emoticones de
teléfono y cambian de forma según el fabricante. Quiero algo **minimalista y
elegante**.

Esto **necesita una dependencia nueva** (una librería de íconos SVG o una fuente
de íconos), así que justificala en el PR: cuál, por qué esa, cuánto pesa el
bundle. Y acordate de la decisión de las siete pestañas de arriba: si la barra
se rehace, se rehace una vez.

---

## Producción

Desplegada al 2026-08-18: migraciones hasta la **`0012`**, 7 áreas protegidas
sembradas, índice `idx_avistamientos_mapa` creado. **Le falta el PR 73.**

**No hay deploy automático.** El workflow que existía apuntaba a EKS y fallaba
siempre; se borró en el PR 75. Redesplegar es a mano.

Si alguna vez se automatiza: la máquina **no tiene IP pública**. Se llega por
una red **ZeroTier** (`10.244.0.0/16`, interfaz `ztbpaiczc3`) y el túnel
Cloudflare solo publica `api.budaicapital.com` y `storage.budaicapital.com`.
Un runner de GitHub no la alcanza. Los caminos son: SSH por Cloudflare Access
con service token, unir el runner a ZeroTier, o un runner self-hosted en la
máquina. **Eso lo decido yo, no lo implementes por tu cuenta.**

Al redesplegar, **nombrá siempre los servicios concretos**: un `build`/`up`
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
```

⚠️ En **zsh** no sirve meter eso en una variable y hacer `$C build ...`: zsh no
parte la variable en palabras y falla con "no such file or directory". Usá el
comando completo, o `${=C}`.

Los contenedores se llaman `chiloe-postgres`, `chiloe-gateway`,
`chiloe-especies-api`, `chiloe-auth`, `chiloe-minio`, `chiloe-redis`,
`chiloe-cloudflared` (tienen `container_name` explícito).

---

## Entorno

- Build de Android: `JAVA_HOME=/usr/lib/jvm/java-17-openjdk` (el default del
  host es Java 26 y el plugin Gradle de RN no parsea esa versión).
- `especies-api` **solo compila dentro de Docker**: al host le faltan Pistache y
  libpqxx. Los tests de gtest se reproducen con
  `docker build --target tester services/especies-api`, que es lo que hace CI.
- `applicationId cl.chiloe.biodiversidad`. El `release` está firmado con la
  clave de debug y `enableProguardInReleaseBuilds = false`.
- La key de Google Maps sale de `GOOGLE_MAP_API` en el `.env` y va a
  `mobile/android/local.properties` como `MAPS_API_KEY`. Ese archivo **no se
  commitea**; el `manifestPlaceholder` de `android/app/build.gradle` la inyecta
  en build time y también lee la variable de entorno del mismo nombre.
- Producción es `donaldchavez@10.244.117.161`, checkout en
  `~/servicios/chiloe-biodiversidad-api`, proyecto compose `chiloe-prod`, env
  **fuera del repo** en `~/.config/chiloe-prod/chiloe.env`. El host viejo
  `10.244.19.205` es el entorno de test. Postgres: usuario `chiloe_prod`, base
  `chiloe_biodiversidad`.
- El CI del backend corre **solo en `pull_request`** desde el PR #70.
- El job `test-especies-api` se cuelga a veces en `Install postgresql-client`
  (apt no responde) y GitHub lo cancela a los 20 min. No es el código:
  `gh run rerun <run-id> --failed` y listo. Ojo con la duración que muestra la
  UI, que **suma los dos intentos** y parece un cuelgue de media hora.

---

## Otros pendientes, más viejos

- **`queryStr` no decodifica nada**: el arreglo del PR 73 fue solo para `bbox`.
  Cualquier búsqueda con espacios o acentos (`?q=zorro%20chilote`) llega mal a
  los demás endpoints. Es un PR chico y aparte; no lo metí en el 73 para no
  ensancharlo.
- **Prueba manual de la cámara en el teléfono**: nunca se hizo del todo. Está en
  [PROMPT_CAMARA_SESION.md](PROMPT_CAMARA_SESION.md) y se solapa con el trabajo 0.
- Fase 9: faltan los PRs **11** (insignias), **12** (pantalla de usuarios del
  panel) y **13** (postular a curar). Del 12 hay una **decisión pendiente
  conmigo**: `users` vive en `auth-service` y las asignaciones a categorías en
  `especies-api`, así que hay que elegir si el panel consulta a los dos o si uno
  expone la vista combinada.
- Fotos para una especie con "hartas fotos": bloqueado esperándome. No bajes
  imágenes de licencia indeterminada a producción.
- Verificación del Paso 3, puntos que faltan: 3 (filtros y paginación),
  5 (visibilidad), 9 (sync del cache de especies). El 7 solo cuando ponga modo
  avión a mano.
- Sin decidir: un PR aparte por la contraseña de Postgres en texto plano que
  `especies-api` imprime al arrancar y queda en los logs del contenedor de
  producción.
