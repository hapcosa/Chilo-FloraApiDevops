# Prompt: continuar la cámara en otra sesión

Pégalo tal cual al abrir la sesión nueva. Está escrito para trabajar **solo** en
el submódulo `mobile/`, en paralelo con la sesión que sigue la Fase 9 en el
backend. **Un agente = un worktree**: si hay otra sesión sobre este repo, esta
va en su propio `git worktree`.

---

## Contexto

App Android React Native bare CLI (`mobile/`, submódulo de
`hapcosa/chiloe-biodiversidad-api`) con módulo nativo de cámara en C++ sobre
NDK Camera2. El PR #31 del repo móvil ya está **mergeado** y trae:

- Toque-para-enfocar (`AF_MODE_AUTO` + `AF_REGIONS`/`AE_REGIONS` + trigger
  CANCEL→START→IDLE; antes quedaba en `CONTINUOUS_PICTURE`, que reenfocaba solo
  y descartaba la región a los pocos frames).
- Controles manuales "pro" (ISO, exposición, dioptrías) alimentados por
  `capabilities()` nativo, con `RangeSlider` propio hecho a `PanResponder`
  —sin `@react-native-community/slider`.
- Selector de galería vía `ACTION_GET_CONTENT` + `startActivityForResult`
  —sin dependencia npm y sin permiso de almacenamiento.
- Visor + disparador para avatar (`PerfilScreen`) y encuentros
  (`MiEncuentroFormScreen`), y pestaña 📷 en la barra de navegación.
- `src/native/previewGeometry.ts`: convierte un toque en la vista a punto
  normalizado de la imagen, deshaciendo el recorte "cover" y la rotación de
  pantalla. Es el espejo de `ChiloeCameraPreviewView.applyPreviewTransform` y
  tiene 5 tests.

Archivos clave:

```
android/app/src/main/cpp/camera/camera_ndk.{hpp,cpp}   # Camera2 NDK
android/app/src/main/cpp/camera/chiloe_camera_jni.cpp  # puente JNI
android/app/src/main/java/.../camera/ChiloeCameraModule.kt
android/app/src/main/java/.../camera/ChiloeCameraPreviewView.kt
src/native/ChiloeCamera.ts        # API JS del módulo
src/native/previewGeometry.ts     # geometría pura, testeable
src/components/RangeSlider.tsx
src/screens/CameraScreen.tsx
```

---

## Lo primero: la verificación manual que quedó pendiente

**Nunca se probó en el teléfono.** La sesión anterior no pudo: el equipo estaba
con bloqueo de credencial y las capturas salían negras. El usuario ya lo
desbloqueó. El APK de release ya está instalado.

Dispositivo: Samsung A53 por adb en red (`192.168.1.7:38015` — reconfirmar con
`adb devices`, la IP puede haber cambiado).

⚠️ **El JDK por defecto del host es openjdk 26.0.2 y el plugin Gradle de React
Native no sabe parsear esa versión** (`IllegalArgumentException: 26.0.2`).
Compilar siempre así, sin meter la ruta en el repo:

```bash
cd mobile/android
JAVA_HOME=/usr/lib/jvm/java-17-openjdk ./gradlew assembleRelease
```

Qué comprobar, con evidencia (captura o log), no de memoria:

1. **Toque-para-enfocar**: tocar un objeto cercano y otro lejano; el marcador
   aparece donde se tocó y el enfoque cambia de verdad. Probar en vertical y en
   apaisado (la rotación de pantalla es justo lo que `previewGeometry` deshace).
2. **Sliders pro**: ISO, exposición y dioptrías mueven la imagen y los límites
   coinciden con lo que reporta `capabilities()`. Si el lente es de foco fijo
   (`focusMaxDiopters === 0`) el slider debe verse deshabilitado, no roto.
3. **Galería**: elegir una foto la trae, le quita el EXIF sensible y reporta las
   dimensiones correctas.
4. **Cámara frontal** en el avatar (`lens: 'front'`).
5. Que la pestaña 📷 no deje la cámara abierta al cambiar de pestaña
   (`useIsFocused` la desmonta; verificar que el LED/preview se apaga).

Si algo falla, arreglarlo es el trabajo de la sesión. **Si no se pudo probar,
decirlo explícitamente** en vez de dar por bueno el código.

---

## Pendiente de producto, ya decidido con el usuario

- **Tras la captura, ofrecer crear un encuentro.** Hoy `CameraScreen` acepta
  `onCapture`, así que el enganche existe; falta la pantalla intermedia que
  pregunte "¿quieres registrar esto como encuentro?" y navegue al formulario con
  la foto ya cargada. Esto está en la Fase 9.0 del plan
  (`docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md`, PR #58 del backend) — coordinar
  para no chocar con la otra sesión.
- **Dejar la cámara pro disponible** (pedido textual del usuario): hoy los
  controles manuales están detrás de un botón. Revisar si conviene recordar la
  preferencia entre sesiones.
- **Advertencia de no molestar a la fauna**: el usuario no quiere motivar a
  perseguir animales, sobre todo del reino `animalia`. La cámara y el flujo de
  encuentros deben orientar a parques y senderos. Redacción y ubicación exacta
  están en el plan de Fase 9.

---

## Reglas innegociables

- Rama por cambio → commits → push → **PR contra `master`** → checks verdes →
  **el merge lo hace el usuario**. Nada de push a `master`, `--force`,
  `reset --hard` ni `--no-verify`.
- Sin dependencias npm nuevas sin justificarlas en el PR. Las dos que se
  evitaron (slider y galería) fueron decisión deliberada.
- Fotos: nunca multipart contra la API. Presigned URL → subida directa →
  notificar la key. EXIF sensible (GPS, serial) fuera salvo opt-in explícito.
- TypeScript estricto, ESLint, Jest. Toda lógica no trivial nueva lleva test.
  La lógica pura va en TS o en C++ testeable sin Android, no en Kotlin.
- Español en documentación, UI y comentarios. Inglés en identificadores.
- **Avisar toda desviación**: cualquier default elegido, aproximación o parte
  del pedido omitida se lista con su razón. Nunca en silencio.
- No commitear: `.idea/`, `diseño/`, keystores, `google-services.json`,
  claves de firma ni `.env` reales.

---

## Comandos

```bash
cd mobile
npm run typecheck      # tsc --noEmit
npm run lint           # 34 warnings de no-void son preexistentes
npm test               # 86 tests / 11 suites al cerrar la sesión anterior
adb devices
adb install -r android/app/build/outputs/apk/release/app-release.apk
```
