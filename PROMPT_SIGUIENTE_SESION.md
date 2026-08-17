# Prompt para la siguiente sesión

Copiá todo lo que sigue como primer mensaje de la sesión nueva.

---

Seguimos con la app móvil de biodiversidad de Chiloé. Trabajás en el submódulo
`mobile/` (`/home/obrero/programacion/Chilo-FloraApiDevops/mobile`), que es su
propio repo git: `hapcosa/chiloe-biodiversidad-mobile`.

## Reglas innegociables

- Rama por cambio → commits → push → PR contra `master` → checks verdes →
  **el merge lo hago yo**. Nada de push a `master`, `--force`, `reset --hard`
  ni `--no-verify`.
- Migraciones numeradas y **nunca** editadas tras mergear.
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
- Este archivo (`PROMPT_SIGUIENTE_SESION.md`, en la raíz del backend) está
  trackeado: actualizalo al cerrar la sesión, en su propio PR.

## Entorno

- Build: `JAVA_HOME=/usr/lib/jvm/java-17-openjdk` (el default del host es Java 26
  y el plugin Gradle de RN falla).
- `applicationId cl.chiloe.biodiversidad`. El buildType `release` está firmado
  con la clave de debug y `enableProguardInReleaseBuilds = false`.
- `src/config/appConfig.ts`: debug → `http://localhost:8080`,
  release → `https://api.budaicapital.com`. Solo el APK release habla con prod.
- Dispositivo por adb: `192.168.1.7:46723`.
  Logs: `adb logcat -d --pid=$(adb shell pidof cl.chiloe.biodiversidad)`
  (un grep por "chiloe" a secas se ahoga en ruido de SurfaceFlinger).
- Los screenshots salen 1080×2400 y se muestran a 900×2000: multiplicá las
  coordenadas por 1.20. El tab "Perfil" está en `1010,2230`; "Probar cámara NDK"
  en `540,1925`; el obturador en `540,1914`.
- Para sacar archivos del cache de la app hay que poner `debuggable true` en el
  buildType release **temporalmente** (no commitearlo) y usar
  `adb exec-out run-as cl.chiloe.biodiversidad cat <ruta>`.

## Estado al cerrar la sesión anterior

### PRs esperando tu merge

- mobile **#26** — icono de la app.
- mobile **#29** — orientación del preview + cierre de la app al volver.
  Checks verdes.
- backend **#56** — DNS.

Mobile #27 y #28 ya están mergeados.

### Trabajo sin commitear en el working tree de `mobile/`

En `android/app/src/main/cpp/camera/camera_ndk.cpp` hay cambios **sin compilar
y sin commitear** que atacan la deformación del preview:

1. `chooseStreamSize()` acepta un parámetro `aspect` y descarta las
   resoluciones cuya relación de aspecto no coincida (tolerancia 0.02).
2. `open()` guarda `impl_->jpegSize` y **resuelve `impl_->previewSize` ahí
   mismo**, exigiendo el aspecto del JPEG, con caída a "cualquier tamaño" si
   ninguna coincide.
3. `setPreviewSurface()` ya no elige el tamaño: usa `impl_->previewSize`.

El motivo de mover la elección a `open()` es que la vista necesita conocer el
tamaño **antes** de crear el `Surface`. **Falta la mitad Kotlin**, que es la
que cierra el arreglo (ver abajo).

## Lo primero que hay que hacer

### 1. Terminar el arreglo de la deformación del preview

En `ChiloeCameraPreviewView.attachIfReady()`, antes de crear el `Surface`:
pedir `sensorGeometry(sessionId)` y llamar
`texture.setDefaultBufferSize(geometry.previewWidth, geometry.previewHeight)`.
Esa es la receta oficial de Camera2 (`Camera2Basic` hace
`setDefaultBufferSize` desde el `SurfaceTexture`, no desde el nativo).
Probablemente convenga sacar el `ANativeWindow_setBuffersGeometry` del C++.

Después: compilar, instalar, medir de nuevo y commitear.

**Lo que ya está medido en el dispositivo, para no repetirlo:**

- La orientación está **bien**. Rotación 0 es la correcta. Lo confirmé
  comparando el preview con el JPEG: ambos muestran la escena igual de derecha.
- La matriz que el productor aplica al `SurfaceTexture` es la **identidad**:
  el productor no rota. (La justificación contraria que había en el código y en
  el PR era falsa y ya está corregida.)
- El JPEG sale perfecto: 4624×3468 con EXIF `orientation=6`, se ve derecho y sin
  deformar. El camino de la foto **no se toca**.
- La deformación del preview es real y se mide con la proporción del mismo
  objeto (un tarro metálico) en foto y preview:

  | versión | proporción del tarro | deformación (1.000 = perfecto) |
  |---|---|---|
  | foto (referencia) | 1.15 | — |
  | preview original (stream 16:9) | 0.68 | 0.59 |
  | preview con stream 4:3 | 0.87 | 0.79 |

- Con el stream forzado a 4:3 el aparato elige **1440×1080** (antes 1920×1080).
- **La causa del 21% que queda**: el buffer entrega un campo de visión casi
  cuadrado (aspecto 0.974) **sin importar qué stream se pida** — o sea,
  `ANativeWindow_setBuffersGeometry` no manda. De ahí la corrección por
  `setDefaultBufferSize`.

Método de medición reproducible (sirve para verificar el arreglo): sacar un
screenshot del preview y una foto de la misma escena sin mover el teléfono,
aplicar el EXIF al JPEG con `PIL.ImageOps.exif_transpose`, y comparar la
proporción del bounding box de un objeto brillante por umbral de luminancia.
También sirve correlacionar perfiles 1-D de columnas y filas para obtener las
escalas horizontal y vertical por separado. No hay numpy en el host.

### 2. El enfoque no funciona

El preview ya pide `AF_MODE_CONTINUOUS_PICTURE`, así que hace falta
toque-para-enfocar con disparo explícito de AF
(`ACAMERA_CONTROL_AF_TRIGGER_START` y regiones `AF_REGIONS`).

### 3. Los cinco puntos del pedido original, todos sin empezar

Del mensaje original: *"la imagen de perfil se toma automáticamente, no te
muestra la escena a capturar y también debería tener la opción de colocar una
imagen cualquiera que uno tenga en el celular, y el problema de la cámara
persiste... también capturar una foto desde la app debería ser un botón que esté
en la barra de navegación ya que si ves una especie la quieres capturar rápido,
hay que mejorar la cámara y que sea configurable en lo posible como la cámara
pro que viene integrada al sistema operativo"*.

1. **Visor con obturador para el avatar.** `PerfilScreen.pickAvatar()` (~línea
   153) hace `openCamera({lens:'front'})` y `capture()` al toque, sin mostrar
   nada. `MiEncuentroFormScreen.tomarFoto()` tiene **exactamente el mismo bug**.
   Los dos deberían usar una pantalla de cámara con visor.
2. **Selector de galería.** Plan acordado: un puente Kotlin con
   `ACTION_GET_CONTENT`, **no** una dependencia npm nueva (el CLAUDE.md del repo
   prohíbe dependencias sin justificar).
3. **Deformación del preview** — ver punto 1 de arriba.
4. **Botón de captura en la barra de navegación.** Va en
   `src/navigation/AppNavigator.tsx`, que hoy tiene los tabs
   `Home | Explorar | Comunidad | Guardados | Perfil` con iconos emoji.
5. **Controles manuales tipo "cámara pro".** Hoy `CameraSession` expone
   `setIso`, `setExposure` y `setFocus`, pero **no hay forma de conocer los
   rangos válidos**. Hay que agregar en C++/JNI un `capabilities()` que exponga
   rango de ISO, rango de exposición y distancia mínima de enfoque.

Te dije que 1, 2, 4 y 5 iban en **un solo PR** (`feat/camara-pro-y-galeria`)
en vez de cuatro apilados, porque todos tocan la misma pantalla de captura
nueva.

## Otros pendientes

- Fotos para una especie con "hartas fotos": bloqueado esperándote. No voy a
  bajar imágenes de licencia indeterminada a producción.
- Verificación del Paso 3, puntos que faltan: 3 (filtros y paginación),
  5 (visibilidad), 6 (captura de foto + PUT presigned), 9 (sync del cache de
  especies). El 7 solo cuando pongas modo avión a mano.
- Sin decidir: un PR aparte por la contraseña de Postgres en texto plano que
  `especies-api` imprime al arrancar, y que queda en los logs del contenedor de
  producción.

## Contexto de infraestructura (por si toca backend)

Producción es `10.244.117.161`. El host viejo `10.244.19.205` (`traderbot`) es
ahora el entorno de test. Tres piezas no viven en el repo y hay que llevarlas a
mano: `~/.config/chiloe-prod/chiloe.env`, las credenciales del túnel en
`~/.cloudflared/<uuid>.json` (+ `cert.pem`), y `CLOUDFLARED_UID`/`GID`.

```bash
cd infrastructure/docker
docker compose --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml -p chiloe-prod up -d
```

⚠️ **Nunca dos `cloudflared` del mismo túnel a la vez**: Cloudflare ve dos
conectores y reparte el tráfico. El cutover es parar el viejo y después levantar
el nuevo. En los `build`/`up` **nombrá siempre el servicio concreto**.

Los datos viven en volúmenes Docker con nombre (`chiloe-prod_postgres_data`,
`chiloe-prod_minio_data`, `chiloe-prod_redis_data`): migrarlos requiere
`pg_dump` y un `tar` del volumen, no copiar un directorio.
