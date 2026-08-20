# Prompt para la sesión siguiente

Copiá todo lo que sigue como primer mensaje de la sesión nueva.
Estado al 2026-08-20, con la Fase 9.4 terminada salvo el merge y **un
redespliegue pendiente**.

---

Seguimos con el sistema de biodiversidad de Chiloé:
`/home/obrero/programacion/Chilo-FloraApiDevops` (backend) y su submódulo
`mobile/`, que es su propio repo (`hapcosa/chiloe-biodiversidad-mobile`).

La sesión anterior cerró los **PR 15 y 16**, que era lo que faltaba de la
Fase 9.4. Los tres PRs están abiertos y en verde, esperando tu merge.

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

- **PR 16 — íconos de navegación** (mobile **PR 40**). Los siete tabs pasaron de
  emojis del sistema a SVG de trazo en `src/components/icons/TabIcons.tsx`, que
  toman el color activo/inactivo del tema —cosa que un emoji no podía hacer,
  porque trae su propio color— y engordan el trazo al enfocarse. Dependencia
  nueva: **`react-native-svg`** (MIT), con los trazos de Lucide (ISC) copiados a
  mano; instalar `lucide-react-native` habría arrastrado más de mil componentes
  al bundle, porque Metro no hace tree-shaking. La truncación se arregló con
  `Comunidad → Gente`, fuente de 10 con tracking apretado y
  `tabBarAllowFontScaling: false`, que era lo que la rompía de nuevo con el
  tamaño de fuente del sistema subido. Se quedaron las siete pestañas.
  Entró también el plural roto del mapa ("1 encuentros").
- **PR 15 — filtros por subgrupo**, en dos repos:
  - backend **PR 82**: migración `0013` con quince subcategorías, la tabla de
    referencia `familia_subgrupo` y el backfill; seed `0003`; `total_especies`
    en `GET /api/v1/categorias`; ADR **#25** en `docs/PLAN_MAESTRO.md`.
  - mobile **PR 41**: segunda fila de chips en la biblioteca, con cache de
    categorías en `sync_state` y `categoria_id` en el cache SQLite de especies.
  - **Falta el PR que suba el submódulo** después de mergear el 41.
- **La decisión del eje**: se reusó `categorias_moderacion` en vez de crear una
  tabla `clases`. Consecuencia aceptada: navegación y curaduría comparten
  registro, así que crear la pestaña "Aves" crea la unidad curable "Aves". La
  regla que queda escrita: la navegación no inventa agrupaciones que no sean
  curables. Está en el ADR #25.
- **Los grupos salen de fuentes, no de la intuición**: los seis de animalia son
  los del Reglamento de Clasificación de Especies del MMA; fungi se parte en
  hongos y líquenes como los nombra el propio MMA; las algas siguen al Museo de
  Historia Natural de Concepción. **Monera quedó sin subgrupos** a propósito:
  sus seis fichas no dan para un selector.
- **"Peces" existe vacío** porque es parte de la partición del MMA. La app
  esconde los subgrupos sin fichas, así que no deja un callejón sin salida —
  para eso está `total_especies`.
- **Ojo con el orden migración/seed**: las migraciones corren **antes** que los
  seeds, así que un backfill en una migración no alcanza a las fichas de un
  entorno nuevo. Por eso el mapeo vive en `familia_subgrupo` y el seed `0003` lo
  reaplica. Ese seed tapa además un hueco viejo de la `0004`, donde en una BD
  recién creada ninguna ficha caía en su categoría "general".
- **Nada se verificó en el teléfono**: no hubo dispositivo en `adb devices` en
  toda la sesión. La APK release del PR 16 quedó compilada.

---

## Trabajo 0 — Redespliegue pendiente (lo hago yo, pedímelo)

Producción tiene mergeado hasta el PR 80 pero **no desplegado**, y a eso se le
suman los PRs 15 y 16 cuando los mergee. Falta:

1. `especies-api` con la portada (PR 77), el difuminado del feed (PR 80) y los
   subgrupos (PR 82, migración `0013` + seed `0003`).
2. **El reload del gateway (PR 79).** Ojo con esto: `nginx.conf` es un
   **bind mount** desde el host (`docker-compose.prod.yml:251`), no está
   horneado en la imagen. Un `up -d --build` **no** hace que el Nginx que ya
   corre relea el archivo. Después del `git pull` hace falta:

   ```bash
   docker exec chiloe-gateway nginx -t && docker exec chiloe-gateway nginx -s reload
   ```

3. **El seed hay que correrlo a mano**: `especies-api-migrate` aplica las
   migraciones, no los seeds. En producción las 103 fichas ya existen, así que
   la `0013` las clasifica sola y el `0003` no debería tener nada que hacer;
   igual conviene correrlo y verificar que dé 0 sin subgrupo.

Cuando me digas que está desplegado, verifico:

```bash
curl -s https://api.budaicapital.com/api/v1/portada | head -c 400
curl -s https://api.budaicapital.com/api/v1/categorias | head -c 600
```

Si la portada sale como el índice del gateway (`{"service":"...","endpoints":[...]}`)
con un 200, el reload no pasó. En `categorias` tienen que venir veinte entradas
con su `total_especies`; si vienen cinco, la `0013` no se aplicó.

---

## Trabajo 1 — Verificar en el teléfono (pendiente de la sesión anterior)

**Enchufá el teléfono**: quedó todo sin ver. Hay que confirmar tres cosas con la
APK release instalada:

1. **Los íconos nuevos**: que los siete se vean como trazos y no como emojis,
   que tomen el verde activo, y sobre todo **que ninguna etiqueta se trunque**.
   Si "Guardados" o "Explorar" siguen cortadas, bajar la fuente a 9 es el
   siguiente paso; está en `styles.tabLabel` de `AppNavigator.tsx`.
2. **La fila de subgrupos** en Explorar: aparece al elegir un reino, desaparece
   con "Todos", y en monera no aparece nunca.
3. **La portada con contenido**, que quedó sin verificar desde el PR 14.

Probar por adb a ciegas sale mal: los taps caen en la pestaña equivocada. Sacá
`screencap` primero y confirmá dónde quedaste con
`adb shell dumpsys window | grep mCurrentFocus`.

---

## Trabajo 2 — Lo que sigue

Con la Fase 9.4 cerrada, los pendientes reales son:

- **Fase 9**: faltan los PRs **11** (insignias), **12** (pantalla de usuarios del
  panel) y **13** (postular a curar). Del 12 hay una **decisión pendiente
  conmigo**: `users` vive en `auth-service` y las asignaciones a categorías en
  `especies-api`, así que hay que elegir si el panel consulta a los dos o si uno
  expone la vista combinada.
- **`queryStr` no decodifica nada**: el arreglo del PR 73 fue solo para `bbox`.
  Cualquier búsqueda con espacios o acentos (`?q=zorro%20chilote`) llega mal a
  los demás endpoints. Es un PR chico y aparte. **Ahora importa más**: la
  biblioteca combina búsqueda de texto con el filtro de subgrupo.
- **Sembrar los subgrupos que falten**: "Peces" está vacío, y las familias que
  no estén en `familia_subgrupo` mandan sus fichas a la categoría "general" del
  reino. Al agregar especies nuevas hay que agregar su familia al mapeo.
- Fotos para una especie con "hartas fotos": bloqueado esperándome. No bajes
  imágenes de licencia indeterminada a producción.
- Verificación del Paso 3, puntos que faltan: 3 (filtros y paginación),
  5 (visibilidad), 9 (sync del cache de especies). El 7 solo cuando ponga modo
  avión a mano.
- Sin decidir: un PR aparte por la contraseña de Postgres en texto plano que
  `especies-api` imprime al arrancar y queda en los logs del contenedor de
  producción.

---

## Producción

Desplegada al 2026-08-19: migraciones hasta la **`0012`**, 7 áreas protegidas
sembradas, 103 especies, 5 reinos. **Le faltan los PRs 77, 79, 80 y 82.**

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
  es del entorno, no del repo. Con `react-native-svg` recién instalada la
  primera build de release tardó **12 minutos**; las siguientes son
  incrementales.
- `especies-api` **solo compila dentro de Docker**: al host le faltan Pistache y
  libpqxx. Ignorá los errores de clang sobre esos headers. Los tests de gtest se
  reproducen con `docker build --target tester services/especies-api`, que es lo
  que hace CI; para correr un subconjunto:
  `docker run --rm --entrypoint /app/build/tests/unit_tests <img> --gtest_filter='*LoQueSea*'`.
- **Para probar una migración de verdad** no hace falta el compose entero: un
  `docker run -d -e POSTGRES_PASSWORD=postgres -e POSTGRES_DB=chiloe_flora -p 55432:5432 postgres:16`
  y después `DB_HOST=localhost DB_PORT=55432 DB_USER=postgres DB_PASSWORD=postgres`
  con `./scripts/migrate.sh` y `./scripts/seed.sh`. Es lo que confirmó que la
  `0013` clasifica las 103 fichas y que reaplicarla no cambia nada.
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
- `npm run lint` en mobile arrastra **48 warnings `no-void`** preexistentes.
  0 errores es el criterio; no salgas a limpiarlos sin PR propio.
- Si agregás un endpoint nuevo, agregá su `location` en `nginx.prod.conf` **y**
  en `nginx.dev.conf`: Nginx lista una por ruta y lo que no matchea cae en
  `location /`, devolviendo el índice del gateway con un **200**.
