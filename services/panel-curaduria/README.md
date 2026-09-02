# Panel de curaduría

Herramienta interna para los curadores del catálogo. Es una SPA estática (React
+ Vite + TypeScript) que se compila **dentro de la imagen del gateway** y se
sirve bajo `/curaduria/`. No es un servicio en ejecución: no hay proceso propio,
puerto propio ni healthcheck propio.

La curaduría no vive en el móvil (ADR #14): los formularios de fichas —atributos
por reino, fuentes, taxonomía— son hostiles en una pantalla de teléfono.

## Qué hace

| Pantalla | Ruta | Quién |
| --- | --- | --- |
| Login | `/curaduria/login` | cualquiera con cuenta |
| Listado de especies (filtros por categoría, estado y texto) | `/curaduria/especies` | curadores |
| Alta / edición de ficha, con publicar y despublicar | `/curaduria/especies/nueva`, `/curaduria/especies/:id` | curador de esa categoría |
| Bandeja de postulaciones a curador | `/curaduria/postulaciones` | solo `admin` |
| Bandeja de avistamientos | `/curaduria/avistamientos` | `admin` o `moderator` |

El formulario de `atributos_especificos` **no** lleva los campos escritos a mano:
se genera en runtime desde `GET /api/v1/schemas/:reino`, es decir desde el mismo
archivo que usa el validador del servidor. Lo que el panel no sabe renderizar se
muestra como aviso y se conserva tal cual, en vez de perderse en silencio.

Las fotos van siempre por presigned URL: el panel pide la URL a
`POST /api/v1/uploads/presign` y hace `PUT` directo al object storage. Nunca
`multipart/form-data` contra la API.

## Desarrollo

```bash
npm ci
npm run dev     # http://localhost:5173/curaduria/
```

El dev server proxya `/api` al gateway (`http://localhost:8080` por defecto;
`PANEL_API_TARGET` lo cambia), así que `make dev` tiene que estar levantado.

```bash
npm run lint
npm test        # vitest: la lógica de schema → campos de formulario
npm run build   # tsc --noEmit + vite build
```

Los tests cubren `src/schema/campos.ts`, que es donde está la lógica real (un
JSON Schema entra, una lista de campos sale). Usan recortes de schema inline a
propósito: importar los archivos de `especies-api` cruzaría servicios y se
rompería con cualquier movimiento de carpetas. La cobertura contra los schemas
reales vive en el test C++ de `AtributosSchemaValidator::schemaDe`.

## Configuración

| Variable | Cuándo | Para qué |
| --- | --- | --- |
| `VITE_S3_PUBLIC_BASE` | **build** | URL pública del object storage, para mostrar las fotos ya subidas. Se pasa como build-arg desde el compose (`S3_PUBLIC_ENDPOINT`). |
| `PANEL_API_TARGET` | dev | Destino del proxy `/api` del dev server. |

`VITE_S3_PUBLIC_BASE` se hornea en el bundle: cambiarla obliga a reconstruir la
imagen del gateway. Es la contrapartida de no tener servidor propio.

## Limitaciones conocidas

- La bandeja de avistamientos **no previsualiza la foto**: el bucket
  `avistamientos-fotos` es privado y `UploadPresignService` solo firma `PUT`, no
  `GET`. Se muestra la clave del objeto. Resolverlo pide presigned GET en la API,
  que es trabajo aparte.
- La sesión vive en `sessionStorage`: sobrevive a un F5, muere al cerrar la
  pestaña. Deliberado — es un panel con permisos de escritura sobre el catálogo.

## Entrar con Google

Quien creó su cuenta en la app con Google no tiene contraseña, así que el login
por correo no le sirve. El panel muestra el botón de Google (Google Identity
Services, cargado desde el CDN de Google la primera vez que se abre el login) y
manda el `credential` a `POST /api/v1/auth/google`, el mismo endpoint que usa la
app móvil.

- El ID de cliente es el **Web** client ID (`VITE_GOOGLE_CLIENT_ID` lo sobrescribe).
  No es un secreto: viaja en el bundle.
- Requisito externo: el dominio donde se sirve el panel debe estar en
  **Authorized JavaScript origins** de ese cliente OAuth en Google Cloud Console
  (`https://api.piedrasdelrayadito.cl` para producción, `http://localhost:5173` para
  desarrollo). Sin eso Google no dibuja el botón.
- El backend no necesitó cambios: `GoogleIDTokenLogin` ya acepta el campo
  `credential` y `GOOGLE_CLIENT_ID` ya admite lista de audiencias separada por comas.
