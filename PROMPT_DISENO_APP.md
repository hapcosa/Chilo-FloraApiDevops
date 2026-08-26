# Prompt para la sesión de diseño de la app

Copiá todo lo que sigue como primer mensaje de la sesión nueva. Esta sesión es
**solo de diseño visual de la app móvil**: no toca el backend, no agrega
funcionalidad, no arregla el mapa.

---

Trabajamos sobre la app móvil del sistema de biodiversidad de Chiloé:
`/home/obrero/programacion/Chilo-FloraApiDevops/mobile`, que es su propio repo
(`hapcosa/chiloe-biodiversidad-mobile`) montado como submódulo.

## El encargo

**La app parece un Google Forms.** Es la crítica, textual, de quien la usa: todo
es una tarjeta blanca con borde gris sobre fondo beige, un campo debajo del
otro, un botón verde al final. Funciona y no se ve *mal*, pero no se ve como
nada en particular: es el aspecto por defecto de un formulario, aplicado a una
app que se supone que trata sobre bosques, turberas y bichos de un archipiélago.

El trabajo es darle una identidad visual propia. **No** es maquillar: es decidir
cómo se ve esta app y aplicarlo de forma consistente.

## Reglas innegociables

- Rama por cambio → commits → push → PR contra `master` → checks verdes →
  **el merge lo hago yo**. Nada de push a `master`, `--force`, `reset --hard`
  ni `--no-verify`.
- No commitear `.idea/`, el directorio sin trackear `diseño/`, keystores,
  contraseñas de firma **ni `android/local.properties`**.
- **Sin dependencias nuevas sin justificarlas en el PR**: cuál, por qué esa, y
  qué le suma al bundle. Ojo con esto en particular: la tentación acá es
  instalar un kit de UI entero. **Metro no hace tree-shaking**, así que un
  `import {Button} from <kit>` puede arrastrar el índice completo. Ya se pagó
  ese aprendizaje una vez: los íconos se resolvieron copiando siete trazos de
  Lucide a mano sobre `react-native-svg` en vez de instalar
  `lucide-react-native`. Ver `src/components/icons/TabIcons.tsx`.
- Avisame explícitamente cada desviación, default elegido u omisión.
  En español, directo.
- **Verificación en el teléfono, no en el emulador.** Un cambio de diseño que no
  se vio en pantalla no está terminado. Ver la sección de entorno.
- Este archivo está trackeado: actualizalo al cerrar la sesión, en su propio PR.

## Dónde está el diseño hoy

- `src/styles/theme.ts` es todo el sistema: nueve colores, cinco tamaños de
  espaciado y los rótulos de reino. **No hay tipografía**: cada pantalla elige
  su `fontSize` a mano. **No hay componentes compartidos**: `src/components/`
  tiene exactamente dos cosas (`AvisoFauna`, `RangeSlider`), así que cada una de
  las dieciocho pantallas se dibujó sus propias tarjetas, sus propios chips y
  sus propios botones con `StyleSheet.create` local. De ahí la uniformidad
  aburrida: no es un sistema, es la misma improvisación repetida dieciocho veces.
- La paleta actual: fondo `#F5F1E8`, superficie blanca, verde `#1F6F50`, un
  ámbar `#D99036` que casi no se usa, borde `#D8D0C3`.
- Los íconos de la barra sí tienen criterio (trazo de 1.6, 2.2 al enfocarse) y
  las etiquetas están calibradas para que entren las siete pestañas en 360dp.
  **Eso no se toca sin motivo**: costó una iteración con el teléfono en la mano.

## Las pantallas, y cuáles importan

Las dieciocho están en `src/screens/`. No las rediseñes todas de una: elegí dos
como piloto, mostrámelas, y recién con el visto bueno seguí con el resto.

Las que definen la identidad, en orden de peso:

1. `HomeScreen` — la portada. Es lo primero que se ve y hoy es un hero verde con
   tres carruseles vacíos.
2. `EspecieDetailScreen` — la ficha de una especie. Es *el* contenido del
   producto y hoy son dos tarjetitas de hábitat y distribución con emojis.
3. `BibliotecaScreen` — el catálogo, con dos filas de chips (reino y subgrupo).
4. `MiEncuentroFormScreen` y `CameraScreen` — el flujo de capturar, que es donde
   más se nota el aire a formulario.
5. `LoginScreen`, `BienvenidaScreen` — la primera impresión.

## Restricciones de contenido, que son del dominio y no negociables

- **Fungi**: donde se hable de comestibilidad va el disclaimer de consultar a un
  experto. No lo escondas detrás de un acordeón bonito.
- **Especies amenazadas**: la ubicación se muestra difuminada a propósito y el
  texto lo dice. Ese aviso no se puede volver decorativo.
- Los textos de UI van en **español**.
- Las fotos son mayormente **emojis de placeholder** todavía: la app no tiene
  fotos reales cargadas. Diseñá para que las tarjetas no se caigan cuando la
  imagen no existe, porque hoy es el caso normal.

## Qué espero de vos, concretamente

1. **Una propuesta antes del código.** Decime qué dirección visual proponés y
   por qué encaja con "divulgación científica sobre un archipiélago del sur",
   con las decisiones de paleta, tipografía, forma (radios, bordes, sombras) y
   densidad. Si proponés cambiar la paleta, mostrame los valores.
2. **Un sistema, no parches**: tipografía y componentes compartidos en
   `src/styles/` y `src/components/`, y las pantallas consumiéndolos. Si al
   terminar cada pantalla sigue con su `StyleSheet` propio, no se hizo el
   trabajo.
3. **Las dos pantallas piloto**, con capturas del teléfono.
4. Y **avisame lo que rompas**: cambiar el sistema de espaciado toca todo.

## Entorno

- Build: `JAVA_HOME=/usr/lib/jvm/java-17-openjdk`. El default del host es Java
  26 y el plugin Gradle de RN revienta con `IllegalArgumentException: 26.0.2`.
  No toques `gradle.properties` por eso: es del entorno, no del repo. La primera
  build de release tarda ~12 min; las siguientes son incrementales (~4 min).
- Tiene que ser **release**: `src/config/appConfig.ts` manda el debug a
  `localhost:8080` y solo el release habla con `api.budaicapital.com`. Si la
  pantalla sale roja con "Unable to load script", instalaste el debug.
- El teléfono se conecta por wifi (`adb devices`). **Es un teléfono en uso**: si
  el foco no está en la app, no vayas dando taps a ciegas — sacá `screencap` y
  confirmá con `adb shell dumpsys window | grep mCurrentFocus`. Ya pasó abrir la
  galería del dueño con tres fotos seleccionadas.
- `npx jest`, `npx tsc --noEmit` y `npm run lint` antes de cada push. El lint
  arrastra **48 warnings `no-void` preexistentes**: el criterio es 0 errores, no
  salgas a limpiarlos.
- La app pide sesión al arrancar. Si te quedás en el login, pedímela: no tenés
  las credenciales.

## Lo que NO es de esta sesión

- El mapa no centra en tu ubicación y se mueve con tirones. Es un pendiente
  real, pero es comportamiento, no diseño.
- Postular a curador desde la app (el backend y el panel ya están; falta la
  pantalla). Es funcionalidad.
- El backend, las migraciones y el redespliegue.

Si algo de eso te parece imprescindible para el diseño, decímelo y lo decido yo;
no lo metas de contrabando en un PR de estilos.
