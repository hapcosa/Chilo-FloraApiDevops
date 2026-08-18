# Prompt para la sesión de mapa + captura

Copiá todo lo que sigue como primer mensaje de la sesión nueva.
Estado al 2026-08-18, después del despliegue de la migración `0012`.

---

Seguimos con el sistema de biodiversidad de Chiloé:
`/home/obrero/programacion/Chilo-FloraApiDevops` (backend) y su submódulo
`mobile/`, que es su propio repo (`hapcosa/chiloe-biodiversidad-mobile`).

Esta sesión tiene **dos trabajos**: dejar el mapa visible en el teléfono, y el
PR 2 de la Fase 9. En ese orden — el primero es corto y desbloquea una
verificación que lleva pendiente desde que se mergeó el mapa.

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

## Trabajo 1 — Ver el mapa en el teléfono

El mapa está mergeado (mobile #37) y **nadie lo vio funcionando nunca**. Sin
key de Google el `MapView` renderiza gris, así que no hay evidencia de que los
círculos caigan donde deben.

### 1.1 Crear la key (esto lo hago yo, pedímelo)

En la consola de Google Cloud, proyecto del backend:

1. Habilitar **Maps SDK for Android** (no "Maps JavaScript API", no
   "Maps Static API" — el binding nativo usa el SDK de Android).
2. Crear una **API key** nueva.
3. **Restringirla**, que es la parte que no se saltea: aplicación Android, con
   el par
   - nombre de paquete: `cl.chiloe.biodiversidad`
   - huella SHA-1 de la clave de **debug** (la que firma también el release
     hoy): `AA:37:42:19:E7:B6:F9:98:2B:E3:37:D0:AC:CD:ED:30:D3:82:77:6F`
4. Restringir además la key **por API**, dejando solo Maps SDK for Android.

Una key sin restringir es facturable por cualquiera que la saque del APK, y el
APK es un ZIP que se abre con `unzip`.

La `GOOGLE_MAP_API` que hay en el `.env` local **no la usa nadie**; no es esta.

### 1.2 Ponerla y compilar

```bash
cd mobile
echo 'MAPS_API_KEY=...' >> android/local.properties   # el archivo ya está en .gitignore
cd android
JAVA_HOME=/usr/lib/jvm/java-17-openjdk ./gradlew :app:assembleRelease
```

`local.properties` **no se commitea**. El `manifestPlaceholder` de
`android/app/build.gradle` la inyecta en build time; también lee la variable de
entorno `MAPS_API_KEY` si no existe el archivo.

**Tiene que ser `assembleRelease`, no debug**: `src/config/appConfig.ts` manda
el debug a `http://localhost:8080` y solo el release habla con
`https://api.budaicapital.com`. Con el APK debug no se ve nada del backend.

### 1.3 Instalar y mirar

```bash
adb devices        # reconfirmá la IP, cambia
adb install -r app/build/outputs/apk/release/app-release.apk
adb logcat -d --pid=$(adb shell pidof cl.chiloe.biodiversidad)
```

Qué mirar en la pestaña **Mapa** (🗺️, entre Capturar y Comunidad):

- Que la capa base sea **satelital** y el chip la alterne a híbrida.
- Que las tres capas se prendan y apaguen por separado: Comunidad, Mis
  encuentros, Áreas protegidas.
- Que aparezcan **7 áreas protegidas** — ya están en producción: PN Chiloé,
  Tantauco, Tepuhueico, Ahuenco, Islotes de Puñihuil, Humedal de Caulín,
  Humedales de Putemún.
- Que los círculos de la comunidad caigan **sobre Chiloé** y no en el Golfo de
  Guinea: si el `bbox` se serializara al revés, el mapa pediría celdas de otro
  lugar del mundo **sin dar error**. Hay test de eso
  (`src/api/__tests__/mapaApi.test.ts`), pero el test no prueba que el servidor
  entienda lo mismo.
- Que el filtro por reino recargue, y que tocar un punto caliente filtre por
  esa especie.

Si algo se ve mal, el arreglo va en su propio PR contra el repo de mobile.

### 1.4 Decisión pendiente conmigo

El mapa quedó como **séptima pestaña**. Siete aprietan la barra. La alternativa
es colgarlo del stack de Comunidad. Decidilo mirándolo en el teléfono, no antes.

---

## Trabajo 2 — PR 2 de la Fase 9

**`feat(camara): ofrecer crear un encuentro al terminar la captura`** *(mobile)*

Es el que cierra el flujo de la cámara. El plan completo está en
[docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md](docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md).

Hoy la pestaña "Capturar" deja la foto en la caché local **sin destino**: se
saca y ahí queda. Tras el disparo hay que mostrar una revisión con la foto y
tres salidas:

- **Crear encuentro** → elegir especie → formulario ya existente.
- **Repetir**.
- **Descartar**.

El selector de especie reusa la búsqueda de `BibliotecaScreen` y **admite
"todavía no sé cuál es"**: un encuentro sin especie que la comunidad identifica
después. La tabla `avistamiento_identificaciones` de la migración `0007` ya
existe para eso, no hay que crear nada en el backend.

Toca: `src/screens/CameraScreen.tsx`, una pantalla nueva de revisión y otra de
selección de especie, `src/navigation/AppNavigator.tsx` y
`src/screens/MiEncuentroFormScreen.tsx` (que debe aceptar una foto ya tomada).

Ojo con lo offline-first: el encuentro tiene que poder encolarse sin red, como
todo lo demás (`src/db/mutationQueue.ts`).

---

## Estado de la Fase 9

Van 9 de 13. El plan es
[docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md](docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md).

| PR | Qué | Estado |
|---|---|---|
| 1 | Quitar emojis de las fichas | ✅ mobile #32 |
| 2 | **Crear encuentro al terminar la captura** | ❌ **es el trabajo 2 de acá** |
| 3 | Contar encuentros en vez de fichas abiertas | ✅ mobile #33 |
| 4 | Advertencia de fauna | ✅ mobile #34 |
| 5 | Bio, profesión y visibilidad en el perfil | ✅ backend #64 |
| 6 | Editar perfil de verdad | ✅ mobile #35 |
| 7 | Encuentros anteriores a la app | ✅ backend #65 + mobile #36 |
| 8 | Mapa satelital de encuentros | ✅ mobile #37 — **sin verificar en teléfono** |
| 9 | Endpoint agregado para el mapa | ✅ backend #66 |
| 10 | Parques y áreas protegidas | ✅ backend #69 |
| 11 | **Insignias** | ❌ sin empezar |
| 12 | **Pantalla de usuarios del panel** | ❌ sin empezar |
| 13 | **Postular a curar** | ❌ sin empezar |

Del PR 12 hay una **decisión pendiente conmigo, no la tomes solo**: el listado
de usuarios cruza dos servicios (`users` vive en `auth-service`, las
asignaciones a categorías en `especies-api`), así que hay que elegir si el panel
consulta a los dos o si uno expone la vista combinada.

---

## Producción: al día

Desplegada el 2026-08-18. **No hace falta tocarla en esta sesión.**

- Migraciones al día hasta la **`0012`**, con las 7 áreas protegidas sembradas.
- Índice `idx_avistamientos_mapa` creado.
- Gateway reconstruido: `/api/v1/areas-protegidas` responde 401 sin token (o
  sea la ruta existe y exige sesión), `/curaduria/` responde 200.
- Respaldo previo al despliegue en
  `~/backups/chiloe_20260818_155343.sql.gz` en la máquina de producción.

Si hiciera falta redesplegar, **nombrá siempre los servicios concretos**: un
`build`/`up` pelado levantaría un segundo `cloudflared` del túnel de Chiloé y
Cloudflare repartiría el tráfico entre dos máquinas. En esa máquina corren tres
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
  libpqxx.
- `applicationId cl.chiloe.biodiversidad`. El `release` está firmado con la
  clave de debug y `enableProguardInReleaseBuilds = false`.
- `mobile/src/config/appConfig.ts`: debug → `http://localhost:8080`,
  release → `https://api.budaicapital.com`.
- Producción es `donaldchavez@10.244.117.161`, checkout en
  `~/servicios/chiloe-biodiversidad-api`, proyecto compose `chiloe-prod`, env
  **fuera del repo** en `~/.config/chiloe-prod/chiloe.env`. El host viejo
  `10.244.19.205` es el entorno de test. Postgres: usuario `chiloe_prod`, base
  `chiloe_biodiversidad`.
- El CI del backend corre **solo en `pull_request`** desde el PR #70. Antes
  disparaba también en `push` y lanzaba dos runs por commit; cuando una copia se
  colgaba, el PR quedaba `BLOCKED` con la otra en verde. Si volvés a ver un PR
  trabado con todo verde, mirá si hay un run colgado y cancelalo.

---

## Otros pendientes, más viejos

- **Prueba manual de la cámara en el teléfono**: nunca se hizo. Está en
  [PROMPT_CAMARA_SESION.md](PROMPT_CAMARA_SESION.md), y se solapa con el
  trabajo 2 de acá: conviene hacerla en la misma pasada.
- Fotos para una especie con "hartas fotos": bloqueado esperándome. No bajes
  imágenes de licencia indeterminada a producción.
- Verificación del Paso 3, puntos que faltan: 3 (filtros y paginación),
  5 (visibilidad), 6 (captura de foto + PUT presigned), 9 (sync del cache de
  especies). El 7 solo cuando ponga modo avión a mano.
- Sin decidir: un PR aparte por la contraseña de Postgres en texto plano que
  `especies-api` imprime al arrancar y queda en los logs del contenedor de
  producción.
