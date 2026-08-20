# Prompt para la sesión de filtros por subgrupo e íconos

Copiá todo lo que sigue como primer mensaje de la sesión nueva.
Estado al 2026-08-20, con todo mergeado y **un redespliegue pendiente**.

---

Seguimos con el sistema de biodiversidad de Chiloé:
`/home/obrero/programacion/Chilo-FloraApiDevops` (backend) y su submódulo
`mobile/`, que es su propio repo (`hapcosa/chiloe-biodiversidad-mobile`).

La sesión anterior verificó el mapa y la captura en el teléfono, cerró el
**PR 14 (portada viva)** de la Fase 9.4 y arregló un leak de coordenadas que
apareció por el camino. Quedan el **PR 15** (filtros por subgrupo) y el
**PR 16** (íconos de navegación), y hay un redespliegue pendiente que hago yo.

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
  imprimen. Desde el host de desarrollo **no hay acceso SSH a producción**:
  pedime a mí los comandos que haya que correr allá.
- Este archivo está trackeado: actualizalo al cerrar la sesión, en su propio PR.

---

## Lo que cerró la sesión anterior

- **Trabajo 0 verificado en el SM-A536E**: el mapa dibuja los círculos de la
  comunidad sobre Chiloé (el `percentDecode` del PR 73 funcionó), el filtro por
  reino recarga, los puntos calientes filtran, el chip alterna Satelital ↔
  Híbrida y salen las 7 áreas protegidas. El flujo de captura → encuentro
  también anda.
- **PR 14 — Portada viva**, en dos repos:
  - backend **PR 77**: `GET /api/v1/portada` con últimas publicadas, últimas
    ediciones y últimos encuentros en una sola llamada. `PortadaService` es un
    compositor sobre los servicios que ya existían, no una capa de datos nueva.
  - mobile **PR 39**: `HomeScreen` reescrita con tres carruseles, cache offline
    en `sync_state` y filtro por reino en cliente. **PR 78** subió el submódulo.
- **La portada no lleva coordenadas, a propósito.** Está escrito en
  `include/models/portada.hpp` y hay un test que lo fija. Una portada que
  devolviera el punto exacto sería la puerta de atrás que deja en nada la
  ofuscación del mapa.
- **PR 79 — ruta del gateway.** Me olvidé de ella en el 77 y la portada devolvía
  el índice del gateway con un **200**, así que la app no veía un error: veía una
  portada vacía. Nginx lista **una `location` explícita por ruta**; lo que no
  matchea cae en `location /`. Si agregás un endpoint nuevo, agregá el bloque en
  `nginx.prod.conf` **y** en `nginx.dev.conf`.
- **PR 80 — leak de coordenadas del feed.** El mapa nunca publica el punto exacto
  de una especie en riesgo, pero `GET /api/v1/avistamientos` devolvía la fila
  entera: mismo dato, otro endpoint. Ahora la BD resuelve si la especie es
  sensible en la misma consulta y `difuminarUbicacion` redondea al centro de la
  celda de ~1 km, borra `precision_metros` y marca `ubicacion_difuminada: true`.
  El autor y quien modera siguen viendo el punto exacto.

---

## Trabajo 0 — Redespliegue pendiente (lo hago yo, pedímelo)

Producción tiene mergeado hasta el PR 80 pero **no desplegado**. Falta:

1. `especies-api` con la portada (PR 77) y el difuminado del feed (PR 80).
2. **El reload del gateway (PR 79).** Ojo con esto: `nginx.conf` es un
   **bind mount** desde el host (`docker-compose.prod.yml:251`), no está
   horneado en la imagen. Un `up -d --build` **no** hace que el Nginx que ya
   corre relea el archivo. Después del `git pull` hace falta:

   ```bash
   docker exec chiloe-gateway nginx -t && docker exec chiloe-gateway nginx -s reload
   ```

Cuando te diga que está desplegado, verificá:

```bash
curl -s https://api.budaicapital.com/api/v1/portada | head -c 400
```

Si sale el índice del gateway (`{"service":"...","endpoints":[...]}`) con un 200,
el reload no pasó. Si sale `ultimas_publicadas`, está bien.

Y en el teléfono (el APK release ya está instalado, basta con reabrir la app):
que la portada muestre los tres carruseles con contenido de verdad, no vacíos.
Si siguen vacíos **y** no aparece el aviso "Sin conexión", es este mismo
problema: el fetch tiene éxito, `data` viene `undefined` y el código cae en
`portadaVacia()` sin poder distinguir "no hay nada publicado" de "el endpoint
no existe".

---

## Trabajo 1 — Lo que falta de la Fase 9.4

El plan está en
[docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md](docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md),
sección **Fase 9.4**. El PR 14 ya está hecho. Quedan dos.

### PR 16 — Íconos en vez de emojis *(mobile)* — **hacelo primero**

Los íconos de la barra son emojis del sistema (`🏠 🔎 📷 🗺️ 👥 🔖 🙋`) puestos
como `<Text>` en `src/navigation/AppNavigator.tsx`. Se ven como emoticones de
teléfono y cambian de forma según el fabricante. Quiero algo **minimalista y
elegante**.

**Ya decidimos**: se quedan las **siete pestañas**, el mapa no se cuelga del
stack de Comunidad. Lo que hay que arreglar acá es la **truncación de las
etiquetas** ("Comun…", "Guarda…") — con siete no entran. Etiquetas más cortas,
tipografía más chica, o solo ícono en las secundarias; decidí vos y mostrámelo
en el teléfono.

Esto **necesita una dependencia nueva** (librería de íconos SVG o fuente de
íconos), así que justificala en el PR: cuál, por qué esa, cuánto pesa el bundle.

Va antes que el 15 porque es solo mobile, no depende de ninguna decisión mía y
deja la barra terminada de una vez.

### PR 15 — Filtros por subgrupo *(backend + mobile)*

Dentro de `animalia`: aves, peces, reptiles, mamíferos, anfibios. Y lo que
corresponda en los otros reinos.

**El eje ya existe**: la migración `0004` creó `categorias_moderacion`
—"subgrupo curable dentro de un reino, sin jerarquía"— y `especies.categoria_id`.
Está backfilleada con cinco categorías "general", una por reino. La
recomendación del plan es **reusarla** en vez de crear una tabla `clases` nueva.

⚠️ **Esto arranca con una decisión mía, no con código.** El costo de reusarla:
acopla el eje de **permisos de curaduría** con el eje de **navegación**. Crear
la categoría "Aves" para que la app filtre significa crear también una unidad de
moderación que alguien puede curar. **Planteámelo con las dos opciones y sus
consecuencias, y esperá mi respuesta antes de escribir código.**

Las subcategorías las crea un admin por la API; hay que sembrarlas.

---

## Producción

Desplegada al 2026-08-19: migraciones hasta la **`0012`**, 7 áreas protegidas
sembradas, 103 especies, 5 reinos. **Le faltan los PRs 77, 79 y 80.**

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
# y el reload del gateway, que el build no hace (bind mount):
docker exec chiloe-gateway nginx -t && docker exec chiloe-gateway nginx -s reload
```

⚠️ En **zsh** no sirve meter eso en una variable y hacer `$C build ...`: zsh no
parte la variable en palabras y falla con "no such file or directory". Usá el
comando completo, o `${=C}`.

Los contenedores se llaman `chiloe-postgres`, `chiloe-gateway`,
`chiloe-especies-api`, `chiloe-auth`, `chiloe-minio`, `chiloe-redis`,
`chiloe-cloudflared` (tienen `container_name` explícito).

---

## Entorno

- Build de Android: `JAVA_HOME=/usr/lib/jvm/java-17-openjdk`. El default del
  host es **Java 26** y el plugin Gradle de RN revienta parseándolo con un
  `IllegalArgumentException: 26.0.2`. No toques `gradle.properties` por esto:
  es del entorno, no del repo.
- `especies-api` **solo compila dentro de Docker**: al host le faltan Pistache y
  libpqxx. Ignorá los errores de clang sobre esos headers. Los tests de gtest se
  reproducen con `docker build --target tester services/especies-api`, que es lo
  que hace CI; para correr un subconjunto:
  `docker run --rm --entrypoint /app/build/tests/unit_tests <img> --gtest_filter='*LoQueSea*'`.
- `applicationId cl.chiloe.biodiversidad`. El `release` está firmado con la
  clave de debug y `enableProguardInReleaseBuilds = false`.
- Tiene que ser **release**: `src/config/appConfig.ts` manda el debug a
  `http://localhost:8080` y solo el release habla con `api.budaicapital.com`.
  Si la pantalla sale roja con "Unable to load script", instalaste el debug.
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
- Probar por adb a ciegas sale mal: los taps caen en la pestaña equivocada.
  Sacá `screencap` primero, y confirmá dónde quedaste con
  `adb shell dumpsys window | grep mCurrentFocus`.

---

## Otros pendientes, más viejos

- **Plural roto**: `mobile/src/screens/MapaScreen.tsx:196` dice "1 encuentros".
  Es de una línea; metelo en el PR 16 que ya toca mobile.
- **`queryStr` no decodifica nada**: el arreglo del PR 73 fue solo para `bbox`.
  Cualquier búsqueda con espacios o acentos (`?q=zorro%20chilote`) llega mal a
  los demás endpoints. Es un PR chico y aparte.
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
