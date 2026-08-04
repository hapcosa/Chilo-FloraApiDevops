# Plan Maestro — Biodiversidad de Chiloé (Backend microservicios + APK Android)

> Documento vivo. Cualquier cambio estructural se discute aquí antes de tocar código.
> Última actualización: 2026-07-29.

---

## 1. Visión y alcance

Plataforma de divulgación científica sobre la biodiversidad de la **Isla de Chiloé**, cubriendo los **cinco reinos** (Animalia, Plantae, Fungi, Protista, Monera). Tiene dos productos:

1. **Backend microservicios** (este repo): catálogo CRUD multi-reino, autenticación, gateway, storage de fotos, despliegue en k3s/minikube.
2. **App móvil Android** (submódulo `mobile/`, React Native): biblioteca consultable de especies, login Google/local, captura de fotos con módulo nativo C/C++ usando NDK Camera2, soporte offline lectura+escritura.

### Objetivos no negociables

- **Pipeline disciplinado**: ningún cambio toca `master` sin pasar por rama → tests → PR → revisión → merge.
- **Paridad dev/prod**: lo que funciona en minikube debe funcionar en la VPS con k3s. Mismos manifiestos K8s.
- **Multi-reino desde el día 1**: el modelo de datos no se diseña pensando solo en plantas (aunque ese era el seed original).
- **Offline-first en móvil**: Chiloé tiene conectividad irregular. La app sirve aunque no haya red.

### Fuera de alcance (por ahora)

- iOS.
- Identificación automática por IA (puede ser fase futura).
- Mapa colaborativo en tiempo real.
- Sistema de comentarios o foros.

---

## 2. Arquitectura general

```
                                  ┌─────────────────────────────────┐
                                  │   App Android (React Native)    │
                                  │   ├─ Módulo nativo cámara (C++) │
                                  │   ├─ Cache SQLite (offline)     │
                                  │   └─ Google Sign-In SDK         │
                                  └────────────────┬────────────────┘
                                                   │ HTTPS
                                                   ▼
┌──────────────────────────────────────────────────────────────────────────┐
│                          API Gateway (Nginx)                             │
│   TLS, rate limit, routing /auth/* → auth-service, /api/* → especies-api │
└──────────────┬─────────────────────────────────────┬─────────────────────┘
               │                                     │
               ▼                                     ▼
   ┌────────────────────────┐         ┌──────────────────────────────┐
   │   auth-service (Go)    │         │   especies-api (C++/Pistache)│
   │   - Registro local     │         │   - CRUD multi-reino         │
   │   - Login local        │         │   - Validación JWT (gateway) │
   │   - Login Google       │◄────────│   - Subida fotos (presigned) │
   │     (verifica idToken) │  JWT    │   - Búsqueda/filtros         │
   │   - Emite JWT propio   │         │                              │
   └────────────┬───────────┘         └──────────────┬───────────────┘
                │                                    │
                └─────────────┬──────────────────────┘
                              ▼
              ┌──────────────────────────────────┐
              │   PostgreSQL  +  Redis (sesiones)│
              └──────────────────────────────────┘
                              │
                              ▼
              ┌──────────────────────────────────┐
              │   MinIO (dev/k3s) / S3 (cloud)   │
              │   Buckets: especies-fotos        │
              └──────────────────────────────────┘
```

### Renombrado completado (Fase 1)

`flora-api` → `especies-api` aplicado en la Fase 1. El binario C++ pasó de `chiloe_flora_api` a `chiloe_especies_api`. El repo GitHub pasó de `Chilo-FloraApiDevops` a `chiloe-biodiversidad-api`.

**Lo que NO se renombró** (decisión consciente, ver §10 ADR #8):

- La base de datos `chiloe_flora` y el usuario `flora_user` quedan como nombres históricos. Renombrarlos rompería volúmenes existentes y requiere migración SQL coordinada con datos. No es bloqueante; el código nuevo lee el nombre desde config.
- El namespace de Kubernetes `chiloe-flora`, el cluster EKS `chiloe-flora-cluster` y el path ECR `chiloe-flora/...` quedan históricos por la misma razón (afectan deploys existentes).

---

## 3. Modelo de datos multi-reino

**Decisión**: tabla base `especies` con columnas comunes + columna `JSONB atributos_especificos` validada por **JSON Schema** según el reino. Razones:

- Permite consultas unificadas (listar, buscar) sin joins múltiples.
- Cada reino puede evolucionar sus campos sin migraciones costosas.
- La validación se hace a nivel app (C++) usando el schema del reino.

### Esquema PostgreSQL

```sql
-- Reinos: enum estable
CREATE TYPE reino_enum AS ENUM ('animalia', 'plantae', 'fungi', 'protista', 'monera');

-- Taxonomía clásica reutilizada en todos los reinos
CREATE TABLE familias (
    id SERIAL PRIMARY KEY,
    reino reino_enum NOT NULL,
    nombre VARCHAR(150) NOT NULL,
    descripcion TEXT,
    UNIQUE(reino, nombre),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE generos (
    id SERIAL PRIMARY KEY,
    familia_id INTEGER NOT NULL REFERENCES familias(id) ON DELETE RESTRICT,
    nombre VARCHAR(150) NOT NULL,
    descripcion TEXT,
    UNIQUE(familia_id, nombre),
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- Especies: la tabla central
CREATE TABLE especies (
    id SERIAL PRIMARY KEY,
    reino reino_enum NOT NULL,
    genero_id INTEGER NOT NULL REFERENCES generos(id) ON DELETE RESTRICT,

    -- Identificación
    nombre_comun VARCHAR(200) NOT NULL,
    nombre_cientifico VARCHAR(200) NOT NULL,
    autor_cientifico VARCHAR(200),           -- ej: "(L.) Mill."

    -- Contenido divulgativo
    descripcion TEXT,
    habitat TEXT,
    distribucion_chiloe TEXT,                -- "Norte de Chiloé", "Islas Desertores", etc.

    -- Estado y trazabilidad
    estado_conservacion VARCHAR(60),         -- IUCN: LC, NT, VU, EN, CR, EW, EX, DD
    endemica BOOLEAN DEFAULT FALSE,
    fuentes JSONB DEFAULT '[]',              -- [{titulo, url, autor, año}]

    -- Geolocalización opcional (centroide de avistamientos representativo)
    geo_lat NUMERIC(9,6),
    geo_lng NUMERIC(9,6),

    -- Campos específicos por reino (validados por JSON Schema)
    atributos_especificos JSONB NOT NULL DEFAULT '{}',

    -- Multimedia: solo referencias a object storage
    foto_portada_key VARCHAR(500),           -- key MinIO/S3
    fotos_keys JSONB DEFAULT '[]',           -- ["bucket/key1", ...]

    -- Curaduría: quién puede editar esta ficha (ver categorías más abajo)
    categoria_id INTEGER REFERENCES categorias_moderacion(id) ON DELETE RESTRICT,

    -- Auditoría
    creado_por INTEGER,                      -- user_id de auth-service
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE(nombre_cientifico)
);

CREATE INDEX idx_especies_reino ON especies(reino);
CREATE INDEX idx_especies_atributos ON especies USING GIN (atributos_especificos);
CREATE INDEX idx_especies_nombre_trgm ON especies USING GIN (nombre_comun gin_trgm_ops);
```

### Categorías de moderación (ADR #11 y #14)

El permiso de edición no cuelga del reino sino de una **categoría de curaduría**: un
subgrupo dentro de un reino (ej. "Aves" dentro de `animalia`). Sin jerarquía: el ADR #11
no la pide y añadirla ahora sería especular.

```sql
CREATE TABLE categorias_moderacion (
    id SERIAL PRIMARY KEY,
    slug VARCHAR(60) NOT NULL UNIQUE,        -- 'aves', 'hongos-comestibles'
    nombre VARCHAR(120) NOT NULL,
    reino reino_enum NOT NULL,
    descripcion TEXT,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Muchos a muchos: un curador cubre varias categorías, una categoría tiene
-- varios curadores. usuario_id/asignado_por son referencias lógicas a
-- `usuarios` del auth-service (sin FK, igual que especies.creado_por).
CREATE TABLE moderador_categorias (
    usuario_id INTEGER NOT NULL,
    categoria_id INTEGER NOT NULL REFERENCES categorias_moderacion(id) ON DELETE CASCADE,
    asignado_por INTEGER,
    asignado_en TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    PRIMARY KEY (usuario_id, categoria_id)
);
```

La migración `0004_categorias_moderacion.sql` siembra una categoría `<reino>-general` por
reino y clasifica ahí las especies existentes, para que ninguna ficha quede sin curador
posible. `especies.categoria_id` nace nullable por eso mismo y se endurecerá a `NOT NULL`
en una migración posterior, cuando ningún entorno tenga huérfanas.

Modelo de permisos resultante:

| Quién | Alcance |
|-------|---------|
| `admin` (rol JWT) | Todo, en cualquier categoría. No necesita asignación. |
| `moderator` (rol JWT) | Moderador **global**, se mantiene tal cual. |
| Curador de categoría | Rol `user` + fila en `moderador_categorias`. |

Aprobar a un curador **no** escribe roles en la BD del `auth-service`: solo inserta la
asignación. Es lo que evita acoplar las dos bases de datos.

`ModeracionService::puedeEditarCategoria(usuarioId, rol, categoriaId)` es el único punto
donde se decide esto, y `especie_controller.cpp` lo consulta con
`requireCuradorDeCategoria()` al crear, editar, tocar fotos y borrar. Detalles que no se
deducen de la tabla:

- El permiso se comprueba **antes** de validar el cuerpo, para que quien no está autorizado
  reciba 403 y no un 400 que le sirva de oráculo del esquema.
- Al **mover** una ficha de categoría hacen falta ambas: la de origen y la de destino. Si no,
  un curador podría sacar fichas de su ámbito hacia uno que no le corresponde.
- Una ficha con `categoria_id` nulo (anterior a la migración `0004`) solo la tocan `admin` y
  `moderator`: no hay categoría sobre la que un curador pueda demostrar permiso.
- Un rol desconocido nunca es global: el default es negar.

### Postulaciones a curador (ADR #14)

Cómo se llena `moderador_categorias`: un usuario cualquiera pide la curaduría de una
categoría y un admin resuelve.

```sql
CREATE TYPE postulacion_estado_enum AS ENUM ('pendiente', 'aprobada', 'rechazada');

CREATE TABLE postulaciones_curador (
    id SERIAL PRIMARY KEY,
    usuario_id INTEGER NOT NULL,             -- referencia lógica al auth-service
    categoria_id INTEGER NOT NULL REFERENCES categorias_moderacion(id) ON DELETE CASCADE,
    texto TEXT NOT NULL,                     -- la evidencia que el admin lee
    estado postulacion_estado_enum NOT NULL DEFAULT 'pendiente',
    revisado_por INTEGER,
    revisado_en TIMESTAMPTZ,
    motivo TEXT,                             -- obligatorio al rechazar
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Se puede reintentar tras un rechazo, pero no acumular pendientes duplicadas.
CREATE UNIQUE INDEX uq_postulaciones_curador_pendiente
    ON postulaciones_curador (usuario_id, categoria_id) WHERE estado = 'pendiente';
```

**Aprobar inserta la fila en `moderador_categorias` dentro de la misma transacción** que
marca la postulación como aprobada: un aprobado sin asignación dejaría al curador sin
permisos y con la solicitud ya cerrada. El `UPDATE ... WHERE estado = 'pendiente'` es
además el guard contra la doble resolución si dos admins actúan a la vez (la segunda
recibe 409).

Aprobar **no** escribe roles en la BD del `auth-service`, coherente con el resto del ADR
#14. El postulante y el revisor salen siempre de la identidad verificada, nunca del
cuerpo: mandar `{"estado":"aprobada","usuario_id":1}` en el `POST` no cambia nada.

### Borrador / publicada (ADR #14)

Una ficha a medio escribir no es contenido público. La migración
`0006_especies_estado.sql` añade a `especies`:

```sql
CREATE TYPE especie_estado_enum AS ENUM ('borrador', 'publicada');

ALTER TABLE especies
    ADD COLUMN estado especie_estado_enum NOT NULL DEFAULT 'publicada',
    ADD COLUMN publicado_por INTEGER,        -- referencia lógica al auth-service
    ADD COLUMN fecha_publicacion TIMESTAMPTZ;

-- Un borrador no arrastra la firma de una publicación anterior.
ALTER TABLE especies ADD CONSTRAINT especies_borrador_sin_publicacion
    CHECK (estado <> 'borrador'
           OR (publicado_por IS NULL AND fecha_publicacion IS NULL));
```

El `DEFAULT 'publicada'` es solo para el backfill: las fichas que ya existían estaban
visibles y esconderlas sería una regresión para las apps instaladas. La API, en cambio,
crea **toda ficha nueva como borrador**.

Reglas de la capa de aplicación:

- El estado solo cambia por `POST /api/v1/especies/{id}/publicar` y `/despublicar`, con el
  mismo permiso por categoría que editar la ficha. `estado` en el cuerpo de un `POST` o
  `PUT` se ignora, y el `UPDATE` del repositorio ni siquiera toca esas tres columnas: un
  cuerpo viejo no puede despublicar sin querer.
- Publicar revalida los `atributos_especificos` contra el JSON Schema del reino: es el
  último punto donde se puede parar una ficha que no cumple.
- El listado y `GET /{id}` filtran por visibilidad **antes** que por cualquier `?estado=`:
  pedir borradores no muestra los ajenos. Un borrador ajeno responde 404 y no 403, para no
  convertir el endpoint en un oráculo de qué fichas se están redactando.
- `admin` y `moderator` ven todo; un curador ve además los borradores de sus categorías.

### Identificación comunitaria de avistamientos (ADR #14)

Quién decide qué especie es un avistamiento. La migración `0007_identificaciones.sql` añade:

```sql
CREATE TYPE grado_identificacion_enum
    AS ENUM ('sin_identificar', 'en_discusion', 'investigacion');

CREATE TABLE avistamiento_identificaciones (
    id SERIAL PRIMARY KEY,
    avistamiento_id INTEGER NOT NULL REFERENCES avistamientos(id) ON DELETE CASCADE,
    usuario_id INTEGER NOT NULL,             -- referencia lógica al auth-service
    especie_id INTEGER NOT NULL REFERENCES especies(id) ON DELETE CASCADE,
    comentario TEXT,
    decisiva BOOLEAN NOT NULL DEFAULT false, -- la hizo un curador de esa categoría
    retirada BOOLEAN NOT NULL DEFAULT false,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Una identificación vigente por persona y avistamiento; cambiar de opinión es
-- retirar la anterior, no editarla, para que el historial quede.
CREATE UNIQUE INDEX uq_avistamiento_identificaciones_vigente
    ON avistamiento_identificaciones (avistamiento_id, usuario_id) WHERE NOT retirada;

ALTER TABLE avistamientos
    ADD COLUMN grado_identificacion grado_identificacion_enum
        NOT NULL DEFAULT 'sin_identificar';
```

La regla vive en `calcularGrado()` (`src/models/identificacion.cpp`), función pura y
testeable sin BD, **no en un trigger**: el umbral va a cambiar con el tamaño de la
comunidad y las migraciones no se editan tras mergear.

- Sin identificaciones vigentes: `sin_identificar`. Con una sola: `en_discusion`.
- Con ≥ 2 vigentes y ≥ 2/3 coincidiendo en la misma especie: `investigacion`, y esa
  especie se fija en `avistamientos.especie_id`. El quórum se evalúa como
  `3 * votos >= 2 * total` para no perder el tercio en la división entera.
- Una identificación **decisiva** cierra el avistamiento sola, sin esperar quórum. Si hay
  varias manda la más reciente.

`decisiva` se persiste en vez de recalcularse consultando `moderador_categorias`: quitarle
la categoría a un curador reescribiría hacia atrás decisiones ya tomadas, y el grado dejaría
de ser función de las filas que se ven. La marca la pone el servidor consultando
`ModeracionService::puedeEditarCategoria()`; mandarla en el cuerpo no hace nada.

El recálculo corre **dentro de la misma transacción** que acaba de insertar o retirar, con
un `SELECT ... FOR UPDATE` sobre la fila del avistamiento: dos personas identificando a la
vez se serializan ahí, y ninguna escribe el grado sobre un conteo viejo.

`avistamientos.especie_id` se escribe al llegar a `investigacion` pero **nunca se limpia**
al bajar de grado: pudo haberla puesto un moderador a mano antes de que existiera este
flujo, y perder acuerdo no es lo mismo que contradecirlo.

El grado es independiente de `estado` (`pendiente`/`aprobado`/`rechazado`), que sigue siendo
moderación de contenido —ocultar una foto inapropiada— y se resuelve por
`PATCH /avistamientos/{id}/moderacion`. Son dos decisiones distintas y las toma gente
distinta.

### Campos sugeridos por reino (en `atributos_especificos`)

Pensé en lo que un usuario de divulgación querría leer y lo que es taxonómicamente honesto:

**Animalia**
- `clase` (Mammalia, Aves, Reptilia, Amphibia, Actinopterygii, Insecta, ...)
- `alimentacion` (`herbivoro` | `carnivoro` | `omnivoro` | `insectivoro` | `detritivoro` | `filtrador`)
- `dieta_detalle` (texto libre)
- `comportamiento` (diurno/nocturno/crepuscular, gregario/solitario, migratorio)
- `tamaño_promedio_cm`, `peso_promedio_g`
- `reproduccion` (`vivíparo` | `ovíparo` | `ovovivíparo`), `epoca_reproductiva`
- `sonido_url` opcional (object storage) — útil para aves y anfibios

**Plantae**
- `tipo_planta` (`arbol` | `arbusto` | `hierba` | `liana` | `epifita` | `helecho` | `musgo`)
- `altura_promedio_m`
- `tipo_hoja` (perenne/caduca, simple/compuesta)
- `floracion_meses` (array de enteros 1–12)
- `fruto` (descripción + comestibilidad)
- `usos_tradicionales` (medicinal, alimentario, maderable, ornamental) — relevante por la cultura huilliche
- `tipo_raiz`, `polinizacion` (anemófila, entomófila, ornitófila)

**Fungi**
- `tipo` (`agaricomiceto` | `liquen` | `moho` | `levadura`)
- `comestibilidad` (`comestible` | `no_comestible` | `tóxico` | `psicoactivo` | `desconocido`) — ¡crítico! marcar siempre
- `simbiosis` (saprófito, micorrízico, parásito, liquenizado)
- `sustrato` (madera, suelo, hojarasca, otro hongo)
- `tipo_himenio` (laminas, poros, dientes, lisos)
- `temporada` (otoño, primavera, todo el año)

**Protista**
- `grupo` (algas pardas/rojas/verdes, protozoos, mohos mucilaginosos)
- `ambiente` (marino, dulceacuícola, terrestre húmedo)
- `morfologia` (unicelular, colonial, talo)
- `tamaño_promedio_mm`
- `importancia_ecologica` (texto)

**Monera (Bacteria/Archaea)**
- `dominio` (`bacteria` | `archaea`)
- `forma` (coco, bacilo, espirilo, vibrio)
- `gram` (positivo, negativo, no aplica)
- `metabolismo` (autótrofo/heterótrofo, aerobio/anaerobio)
- `relevancia_chiloe` (texto: rol en suelos turbosos, manglares, etc.)

### Sugerencias adicionales para todos los reinos

- **`avistamientos`** (tabla separada): registros que sube la app móvil con geolocalización + foto + usuario. Permite mapear distribución real sin contaminar la ficha curada.
- **`nombres_locales`** (tabla): nombres en mapudungun/huilliche y otros nombres comunes locales. Chiloé tiene riqueza cultural específica.
- **Versión y revisión**: campo `revisado_por` y `fecha_revision` para distinguir fichas curadas de fichas crowdsourced.

---

## 4. Storage de fotos (MinIO/S3)

**Decisión**: object storage S3-compatible. MinIO en minikube y k3s (dev/staging). S3 real (o S3-compatible de Hetzner/DO) en prod si se migra.

### Flujo de subida

1. App pide a `especies-api` una **URL presigned PUT** (`POST /api/v1/uploads/presign`).
2. App sube directo a MinIO/S3 con esa URL (no pasa por la API → no satura Pistache).
3. App notifica a la API la clave (`PATCH /api/v1/especies/:id { foto_portada_key, fotos_keys }`).
4. La API valida que la clave existe y pertenece al bucket esperado antes de aceptarla.

### Buckets

- `especies-fotos` — fichas curadas, públicas vía CDN.
- `avistamientos-fotos` — fotos de usuarios, requieren moderación antes de ser públicas.

### Procesamiento

- **Stripping de EXIF**: el módulo nativo de cámara escribe metadatos mínimos (sin GPS, sin device serial) salvo lo que el usuario explícitamente quiere compartir.
- **Thumbnails**: generación lazy en un worker (fase 2). Por ahora la app puede pedir resoluciones via query string si el storage soporta transformación (S3 no lo hace nativo; MinIO + Imgproxy sí).

---

## 5. Autenticación

**Decisión**: el cliente React Native usa Google Sign-In SDK → envía `idToken` al `auth-service` → el servicio verifica el token contra Google → crea/asocia usuario → emite JWT propio. El mismo flujo de JWT propio se usa para login email/contraseña.

### Endpoints (en `auth-service`)

```
POST /api/v1/auth/register        { email, password, nombre }
POST /api/v1/auth/login           { email, password }
POST /api/v1/auth/google          { id_token }       ← nuevo
POST /api/v1/auth/refresh         { refresh_token }
GET  /api/v1/auth/profile         (Bearer JWT)
PUT  /api/v1/auth/profile         (Bearer JWT)
POST /api/v1/auth/logout          (Bearer JWT)
```

### Modelo de usuario

```sql
CREATE TABLE usuarios (
    id SERIAL PRIMARY KEY,
    email VARCHAR(255) NOT NULL UNIQUE,
    password_hash VARCHAR(255),                  -- NULL si solo Google
    google_sub VARCHAR(255) UNIQUE,              -- "subject" del idToken
    nombre VARCHAR(150),
    foto_url TEXT,
    rol VARCHAR(30) NOT NULL DEFAULT 'usuario',  -- usuario | curador | admin
    created_at TIMESTAMPTZ DEFAULT NOW()
);
```

- `rol = curador` puede crear/editar fichas oficiales.
- `rol = usuario` solo puede subir avistamientos pendientes de moderación.

### JWT

- Firmado con HS256 (clave en Secret de K8s).
- `exp` 15 min, refresh token 30 días en Redis.
- El gateway Nginx valida JWT en `/api/*` (módulo `auth_request` apuntando al endpoint `/auth/verify` del auth-service) antes de pasar a `especies-api`.

---

## 6. App móvil (submódulo `mobile/`)

### Estructura del submódulo

```
mobile/                           # git submodule → chiloe-biodiversidad-mobile
├── android/
│   ├── app/
│   │   └── src/main/cpp/         # Módulo nativo cámara C++
│   │       ├── camera_ndk.cpp    # NDK Camera2 (AImageReader, ACameraDevice)
│   │       ├── jpeg_writer.cpp   # Escritura JPEG/HEIF
│   │       ├── exif_minimal.cpp  # EXIF mínimo controlado
│   │       └── CMakeLists.txt
│   └── ...
├── src/
│   ├── api/                      # Cliente HTTP a backend
│   ├── auth/                     # Google Sign-In + JWT storage (Keychain)
│   ├── db/                       # SQLite (cache offline)
│   ├── screens/
│   │   ├── BibliotecaScreen.tsx  # Lista filtrable por reino
│   │   ├── EspecieDetailScreen.tsx
│   │   ├── CameraScreen.tsx      # Llama al módulo nativo
│   │   ├── AvistamientoFormScreen.tsx
│   │   └── auth/
│   ├── sync/                     # Cola de mutaciones offline
│   └── native/CameraModule.ts    # Bridge JS ↔ C++
├── package.json
└── README.md
```

### Módulo nativo de cámara (C++ + NDK Camera2)

**Alcance**: NDK Camera2 con controles manuales (ISO, exposición, foco, white balance), captura JPEG/HEIF, sin RAW por ahora.

Características:

- Abre la cámara trasera de mayor resolución usando `ACameraManager`.
- Permite cambiar `CONTROL_AE_MODE`, `CONTROL_AF_MODE`, `SENSOR_SENSITIVITY`, `SENSOR_EXPOSURE_TIME` desde JS.
- Captura con `AImageReader` en `AIMAGE_FORMAT_JPEG`.
- Borra EXIF sensible antes de escribir el archivo.
- Devuelve la ruta del archivo al JS para subirlo vía presigned URL.

API JS expuesta:

```ts
NativeModules.ChiloeCamera.openCamera(opts: { lens: 'back' | 'front' }): Promise<CameraSession>
session.setIso(iso: number): Promise<void>
session.setExposure(ms: number): Promise<void>
session.setFocus(distance: number | 'auto'): Promise<void>
session.capture(): Promise<{ filePath: string; width: number; height: number }>
session.close(): Promise<void>
```

### Offline (lectura + escritura)

- **Cache**: SQLite con `react-native-quick-sqlite`. Replica subset de `especies` que el usuario descargó.
- **Cola de mutaciones**: cualquier creación de avistamiento estando offline se persiste en tabla `pending_mutations` con un `client_id` UUID. Cuando hay red, un worker en background hace replay y mapea el `client_id` al `id` real devuelto por el backend.
- **Conflictos**: para avistamientos (append-only) no hay conflictos. Para ediciones de perfil de usuario, last-write-wins con timestamp del cliente.

### Login

- Google Sign-In SDK oficial de Google (`@react-native-google-signin/google-signin`).
- Login local con email/password.
- JWT y refresh token guardados en `EncryptedSharedPreferences` (Android Keystore).

---

## 7. CI/CD

Disciplina del pipeline (innegociable):

```
desarrollador → git checkout -b feat/xxx
              → cambios
              → git push → GitHub Actions corre tests
              → abre PR contra master
              → tests + lint + build de imágenes deben pasar
              → revisión humana
              → merge a master
              → workflow de deploy se dispara
              → build de imágenes finales → push a registry
              → kubectl apply en k3s
```

### Workflows GitHub Actions

**`.github/workflows/test.yml`** (en PRs y push a ramas):
- Job `especies-api-test`: build C++ con cmake en contenedor, corre gtest.
- Job `auth-service-test`: `go test ./...` con cobertura.
- Job `mobile-test` (en el submódulo): `npm test` + lint, build del APK debug.
- Job `lint`: clang-tidy para C++, golangci-lint para Go, eslint para RN.
- Job `integration`: levanta `docker-compose.dev.yml`, corre el postman/newman.

**`.github/workflows/deploy.yml`** (en merge a master):
- Build imágenes Docker `especies-api` y `auth-service`.
- Push a GHCR (`ghcr.io/hapcosa/...`).
- `kubectl apply -k infrastructure/kubernetes/prod` contra k3s vía kubeconfig en Secret.
- Smoke test contra `/health`.

**Protecciones de rama**:
- `master` requiere PR aprobado y status checks verdes.
- No se permite push directo.

### Tests por servicio

- **`especies-api` (C++)**: gtest. Cobertura mínima: controllers + repositories. Tests de integración con PostgreSQL real en contenedor (testcontainers-cpp o `docker-compose.test.yml`).
- **`auth-service` (Go)**: `go test` con mocks para Google. Tests de integración con Postgres + Redis reales.
- **`mobile`**: Jest para lógica JS. Detox para flujos críticos (login, ver ficha, capturar foto, subir avistamiento offline). El módulo nativo C++ se testea con gtest en CI host (no Android) para la lógica pura, e2e con Detox para la integración Android.

---

## 8. Despliegue

### Local (minikube)

```bash
make minikube-setup
make minikube-deploy
```

Manifiestos en `infrastructure/kubernetes/base/` + overlay `dev/`.

### Producción (VPS con k3s)

**Stack**:
- VPS Linux (Hetzner/DigitalOcean/Contabo, mínimo 4 vCPU / 8 GB RAM).
- k3s single-node inicialmente, multi-node cuando crezca.
- Traefik (incluido en k3s) como ingress, con cert-manager + Let's Encrypt.
- Postgres y MinIO como StatefulSets con PVC en disco del host.
- Backups diarios: `pg_dump` + `mc mirror` a un bucket externo (Backblaze B2 es barato).

**Diferencia con minikube**: solo el overlay `prod/` (réplicas, recursos, dominio, secrets reales). El resto se hereda de `base/`.

### Por qué k3s y no Docker Compose

Mantener paridad con minikube. Aprendes Kubernetes una sola vez. Si más adelante migras a EKS/GKE, solo cambias el overlay.

---

## 9. Hoja de ruta por fases

### Fase 0 — Preparación (esta semana)

- [ ] Crear `docs/PLAN_MAESTRO.md` (este archivo) ✅
- [ ] Crear `CLAUDE.md` en raíz ✅
- [ ] Activar branch protection en `master`.
- [ ] Crear repo separado `chiloe-biodiversidad-mobile` y añadirlo como submódulo en `mobile/`.

### Fase 1 — Migración multi-reino backend (2–3 semanas)

- [x] Renombrar `flora-api` → `especies-api` (PR aparte). ✅
- [x] Sistema de migraciones SQL planas + `scripts/migrate.sh` con tracking en `schema_migrations`; eliminar `CREATE TABLE` embebidos en C++; integrar en CI y compose. (PR aparte) ✅
- [x] Migración SQL multi-reino (`0002_multi_reino.sql`): `reino_enum`, expandir `especies` con `atributos_especificos JSONB`, foto_portada_key, fotos_keys, autor_cientifico, distribucion_chiloe → distribucion_chiloe, fuentes, geo_lat/lng, creado_por, revisado_por, fecha_revision, timestamps + trigger updated_at; familias `UNIQUE(reino, nombre)`; generos `UNIQUE(familia_id, nombre)`; índices GIN sobre atributos y pg_trgm sobre nombre_comun. ✅
- [x] Refactor C++ multi-reino: type `Reino` con helpers, modelos `Especie`/`Familia`/`Genero` con los nuevos campos, repositorios usando las nuevas columnas, services con `getByReino` y `findByNombre(reino, …)`/`findByNombre(familia_id, …)`, controllers que aceptan filtro `?reino=X`. La validación de `atributos_especificos` por reino aún es opaca (siguiente PR). ✅
- [x] JSON Schemas por reino en `services/especies-api/config/schemas/` + `AtributosSchemaValidator` (nlohmann/json-schema-validator). Fungi exige `comestibilidad` por riesgo sanitario; Monera exige `dominio`. `additionalProperties: false` en todos. Validación funcional probada con 5 casos (válidos + 3 tipos de fallo). El job CI ahora hace `docker build` para garantizar paridad. ✅
- [x] Endpoints `/api/v1/especies` con filtros + paginación: `?reino`, `?genero_id`, `?familia_id`, `?conservacion`, `?endemica`, `?q` (ILIKE en nombre_comun + nombre_cientifico, acelerado por índice GIN pg_trgm), `?limit` (max 200), `?offset`, `?orderby` (whitelist), `?orderdir`. Response incluye `pagination.total`. Endpoint legacy `/api/especies/search/genero` eliminado (lo cubre `?genero_id=`). ✅
- [x] Tests gtest sobre cada reino: `services/especies-api/tests/` con target CMake opcional (`-DBUILD_TESTS=ON`), stage `tester` en el Dockerfile y paso en CI. Cubre `reino.cpp` (serialización/parseo) y `AtributosSchemaValidator` validando atributos_especificos por reino contra los JSON Schemas reales (casos válidos por reino, Fungi exige `comestibilidad`, Monera exige `dominio`, `additionalProperties:false`, enum/tipo/rango inválidos). ✅
- [x] Cleanup: eliminado el cluster `user`/`auth` muerto de especies-api (13 archivos: `postgres_user_repository`, `user_repository`, `user_service`, `models/user`, `auth_middleware`, `tokenutils`, `password_utils`). Ninguno compilaba ni se incluía; auth vive en auth-service y el gateway valida el JWT (§5). ✅

### Fase 2 — Storage de fotos (1–2 semanas)

- [x] Desplegar MinIO en compose dev y en manifiestos K8s base. Incluye buckets `especies-fotos` y `avistamientos-fotos` inicializados por servicio/job de bootstrap. ✅
- [x] Endpoint `POST /api/v1/uploads/presign` en `especies-api`: genera presigned URLs `PUT` S3 SigV4 para `especies-fotos` y `avistamientos-fotos`, firmando `Content-Type` y devolviendo la key que debe persistirse luego. ✅
- [x] Validación de claves al hacer PATCH de fotos: `PATCH /api/v1/especies/:id/fotos` consulta object storage con `HEAD` firmado y solo persiste keys existentes en `especies-fotos`. ✅
- [x] Bucket policies: `especies-fotos` con anonymous `download`; `avistamientos-fotos` con anonymous `none` para mantenerlo restringido hasta moderación. ✅

### Fase 3 — Auth con Google (1 semana)

- [x] Endpoint `POST /api/v1/auth/google` en auth-service. ✅
- [x] Tabla `usuarios` con `google_sub` nullable para asociar login Google sin romper login local por email/password. ✅
- [x] Verificación de `idToken` contra `https://oauth2.googleapis.com/tokeninfo`, validando `aud`, `iss`, `exp`, `sub` y `email_verified`. ✅
- [x] Reutilizar emisión de JWT existente. ✅

### Fase 4 — App móvil base (3–4 semanas)

- [ ] Repo `chiloe-biodiversidad-mobile`: submódulo registrado en `.gitmodules`, falta clonar/publicar el repo remoto y dejar un commit de submódulo. En este entorno quedó scaffold local en `mobile/`.
- [x] Scaffold React Native 0.86 bare Android (no Expo) con TypeScript estricto y base nativa Android preparada para NDK; falta generar/verificar Gradle wrapper en el repo móvil definitivo. ✅
- [x] Pantallas: Biblioteca, Detalle, Login, Perfil. ✅
- [x] Cliente API + manejo de JWT con almacenamiento seguro vía Keychain. ✅
- [x] Cache SQLite + sincronización inicial de especies. ✅

### Fase 5 — Cámara nativa C++ (2–3 semanas)

- [x] Esqueleto NDK Camera2 (abrir cámara, capturar JPEG) implementado en `mobile/android/app/src/main/cpp/`; pendiente validar en dispositivo real cuando exista Gradle wrapper/dependencias. ✅
- [x] Controles manuales (ISO, exposición, foco) cableados desde JS hasta `ACaptureRequest`; pendiente calibración por modelo de cámara. ✅
- [x] Bridge JNI + módulo React Native (`ChiloeCameraModule`). ✅
- [x] Pantalla `CameraScreen` que usa el módulo sin preview para MVP de captura. ✅

### Fase 6 — Avistamientos y offline writes (2 semanas)

- [x] Tabla `avistamientos` en backend + endpoints `POST/GET/PATCH /api/v1/avistamientos`; crea registros pendientes y valida `foto_key` contra `avistamientos-fotos`. ✅
- [x] Cola de mutaciones en SQLite móvil para crear avistamientos offline. ✅
- [x] Worker de sincronización al recuperar red. ✅
- [x] Moderación básica: `PATCH /api/v1/avistamientos/:id/moderacion` aprueba o rechaza con motivo obligatorio para rechazo. ✅

### Fase 7 — VPS y producción (1 semana)

- [x] Preparación local: overlay Kustomize k3s producción con Traefik/cert-manager y guía `docs/deployment/K3S_PRODUCCION.md`. ✅
- [ ] Provisionar VPS, instalar k3s.
- [ ] Configurar Traefik + cert-manager con dominio real.
- [ ] Migrar secrets, primer deploy.
- [ ] Configurar backups.

### Fase 8 — Pulido y carga de contenido

- [ ] Importación inicial de ~50 especies por reino (CSV → script).
- [ ] Revisión por un curador.
- [ ] Beta cerrada de la APK.

### Fase 8b — Moderación real, permisos por categoría y encuentros privados

- [x] Cerrar el hueco de autorización real gateway↔especies-api: `X-User-Role` propagado
      por `auth-service`, `auth_request` real en nginx (dev y overlay K8s base) para
      especies/familias/generos/avistamientos, y `especies-api` deja de confiar en
      `creado_por`/`revisado_por`/`moderado_por` del cuerpo del cliente — ahora vienen de la
      identidad verificada (`RequestIdentity`, headers `X-User-Id`/`X-User-Role`). Mutaciones
      de especies y moderación de avistamientos exigen rol `admin` o `moderator`. ✅
- [x] Moderación por categoría (muchos a muchos): tablas `categorias_moderacion` +
      `moderador_categorias` y `especies.categoria_id` (migración
      `0004_categorias_moderacion.sql`), con `GET /api/v1/categorias` para cualquier sesión
      y `POST`/`PUT`/`DELETE` solo para `admin`. ✅
- [x] Restringir edición/fotos de especies según la categoría asignada al moderador (no solo
      el rol admin/moderator genérico): `ModeracionService::puedeEditarCategoria()` y el guard
      `requireCuradorDeCategoria()` en `especie_controller.cpp` (crear, editar, fotos y
      borrar), más los endpoints de asignación de curadores
      (`POST`/`DELETE /api/v1/categorias/{id}/moderadores/{usuarioId}` para `admin` y
      `GET /api/v1/moderadores/{usuarioId}/categorias` para `admin` o el propio usuario). ✅
- [x] Postular a curador desde la app (`postulaciones_curador`, migración
      `0005_postulaciones_curador.sql`); un admin aprueba o rechaza vía
      `PATCH /api/v1/postulaciones/{id}`, y aprobar inserta la asignación en
      `moderador_categorias` en la misma transacción. ✅
- [x] Especies con estado `borrador`/`publicada` (migración `0006_especies_estado.sql`): toda
      ficha nueva nace borrador y se publica con `POST /api/v1/especies/{id}/publicar`
      (`/despublicar` para el camino inverso), con el mismo permiso por categoría que
      editarla. El `GET` público filtra los borradores —es lo que impide que lleguen al cache
      SQLite del móvil— y el curador ve los de sus categorías. ✅
- [x] Panel web de curaduría (`services/panel-curaduria/`, React + Vite + TypeScript),
      compilado dentro de la imagen del gateway y servido bajo `/curaduria/` con el mismo JWT
      del `auth-service` (ADR #15). Pantallas: login, listado filtrable por categoría/estado,
      formulario de especie con `atributos_especificos` generado desde el JSON Schema del
      reino (`GET /api/v1/schemas`), subida de fotos por presigned URL, bandeja de
      postulaciones (admin) y bandeja de avistamientos. La curaduría no vive en el móvil. ✅
- [x] Identificación comunitaria de avistamientos estilo iNaturalist (migración
      `0007_identificaciones.sql`): `POST`/`GET /api/v1/avistamientos/{id}/identificaciones` y
      `DELETE .../{idIdentificacion}` para retirarla. El grado
      (`sin_identificar`/`en_discusion`/`investigacion`) se recalcula en la capa de servicio
      tras cada cambio, con quórum de 2/3 y voto decisivo del curador de la categoría. ✅
- [ ] Avistamientos privados por defecto ("mis encuentros"): visibilidad
      `privado`/`publico`, endpoint para que el dueño comparta un encuentro a la moderación
      pública, UI móvil completa (hoy la cola offline/sync existe pero no hay pantalla).
- [ ] Compartir un encuentro a Instagram/Facebook Stories (intents nativos, sin red social
      propia).
- [ ] Perfil con avatar (subida vía presigned URL, bucket `perfiles-fotos`).
- [ ] Cámara con preview real (hoy es MVP sin preview): `Surface` de un `TextureView` nativo
      expuesta vía JNI al `ACameraCaptureSession` existente.

---

## 10. Decisiones técnicas registradas (ADRs cortos)

| # | Decisión | Alternativas | Por qué |
|---|----------|--------------|---------|
| 1 | Tabla base + JSONB para multi-reino | Tablas hijas / 5 tablas | Flexibilidad, queries unificadas, validación con JSON Schema |
| 2 | MinIO/S3-compatible para fotos | BYTEA / volumen | Escala, no hincha BD, mismo flujo dev/prod |
| 3 | Submódulo git para `mobile/` | Carpeta monorepo | Versionado independiente, CI separado, releases de APK aparte |
| 4 | Google Sign-In SDK → idToken → Go verifica → JWT propio | OAuth completo en backend / Firebase | Estándar OIDC, control de JWT, sin lock-in |
| 5 | NDK Camera2 con controles manuales (JPEG/HEIF), sin RAW | CameraX puro / RAW+HDR | Aprovecha cámara sin la complejidad de DNG |
| 6 | k3s en VPS | Docker Compose / EKS | Paridad con minikube, costo bajo, escalable |
| 7 | Offline lectura+escritura | Solo lectura / Solo online | Realidad de conectividad en Chiloé |
| 8 (2026-05-20) | Renombrar servicio `flora-api` → `especies-api` y binario `chiloe_flora_api` → `chiloe_especies_api`, pero **NO** la DB `chiloe_flora`, el usuario `flora_user`, el namespace K8s `chiloe-flora`, el cluster EKS `chiloe-flora-cluster` ni el path ECR `chiloe-flora/...` | Renombrar todo / no renombrar nada | El servicio necesita un nombre que refleje el alcance multi-reino, pero renombrar la DB y el namespace rompería volúmenes y deploys existentes y obliga a migración SQL coordinada. Aceptamos la inconsistencia "servicio = especies-api, DB = chiloe_flora" como deuda histórica documentada. |
| 9 (2026-07-15) | App móvil inicia como React Native CLI bare, no Expo, con Android primero | Expo / monorepo web-mobile | El módulo NDK Camera2 de Fase 5 necesita control de proyecto nativo y releases APK independientes en el submódulo `mobile/`. |
| 10 (2026-07-25) | Autorización real gateway→especies-api vía `auth_request` de nginx + headers `X-User-Id`/`X-User-Role` confiables; `especies-api` deja de aceptar `creado_por`/`revisado_por`/`moderado_por` del cuerpo del cliente | Validar JWT directamente en el C++ / seguir sin validación | Se detectó que `especies-api` no verificaba identidad en ninguna mutación: cualquiera podía autoatribuirse como cualquier usuario o "aprobar" cualquier avistamiento con solo cambiar el JSON. `auth-service` ya tenía el endpoint `/auth/verify` pensado para `auth_request` pero nginx nunca lo usaba. Reutilizar `auth_request` evita duplicar lógica de verificación de JWT en C++. |
| 11 (2026-07-25) | Moderación por categoría muchos-a-muchos (`categorias_moderacion` + `moderador_categorias`) en vez de solo el rol genérico `moderator` | Un solo rol moderator sin subdivisión / roles fijos por reino (5 roles) | El usuario pidió que un moderador pueda especializarse en subgrupos (ej. "Aves" dentro de Animalia) y que varios moderadores puedan compartir una categoría. Una tabla de asignación muchos-a-muchos es más flexible que roles fijos y permite que reinos poco documentados usen una sola categoría "catch-all". |
| 12 (2026-07-25) | Avistamientos privados por defecto (`visibilidad='privado'`), con acción explícita del dueño para publicarlos, en vez de una tabla nueva para "mis encuentros" | Tabla/flujo separado para encuentros personales | El usuario confirmó que "mis encuentros" es el mismo concepto que los avistamientos ya construidos en Fase 6 (Fase 6 los hizo públicos por defecto). Reusar la tabla y toda la cola offline/sync ya construida en mobile evita duplicar trabajo. |
| 13 (2026-07-29) | En el host compartido actual, desplegar con **Docker Compose + túnel Cloudflare propio** (`infrastructure/docker/docker-compose.prod.yml`), no con k3s. La decisión #6 (k3s) sigue vigente para un VPS dedicado | Instalar k3s junto al Docker existente / no desplegar | El host ya sostiene ~20 contenedores de otro negocio en producción real. El Traefik que k3s trae por defecto reclama los puertos 80/443 —que en este host nadie ocupa porque la salida es por Cloudflare Tunnel— y su containerd conviviría mal con el Docker que corre esas cargas. El riesgo recae sobre un negocio ajeno a este proyecto y la ganancia sería paridad con unos manifiestos que este host no usa. Los manifiestos de `infrastructure/kubernetes/` quedan intactos como camino de migración. Detalle operativo en [docs/deployment/PRODUCCION_DOCKER_CLOUDFLARE.md](deployment/PRODUCCION_DOCKER_CLOUDFLARE.md). |

| 14 (2026-08-03) | Curaduría estilo iNaturalist sobre el ADR #11: (a) se llega a curador **postulando** desde la app y un admin aprueba; (b) las especies tienen estado **borrador → publicada** y el curador publica dentro de su categoría sin pedir permiso; (c) la curaduría vive en un **panel web separado**, no en el móvil; (d) los avistamientos se resuelven por **identificación comunitaria** con voto decisivo del curador. Un curador es un usuario con rol `user` + fila en `moderador_categorias`; aprobarlo **no** escribe roles en el `auth-service` | Nombrar curadores solo a mano desde la BD / meter la curaduría en la app móvil / dejar la identificación de avistamientos a un solo moderador | Postular es lo único que escala sin que el admin conozca personalmente a cada experto. El borrador evita que una ficha a medio escribir llegue al público y al cache SQLite del móvil. El panel web separado mantiene la app enfocada en divulgación y captura, y da formularios largos (atributos por reino, fuentes) que en un teléfono son hostiles. La identificación comunitaria aprovecha a la gente que sabe pero no cura, y el voto decisivo del curador hace que el sistema funcione mientras la comunidad sea chica y el quórum no se cumpla nunca. No escribir roles cruzados es lo que evita acoplar la BD de `especies-api` con la de `auth-service`. |

| 15 (2026-08-03) | El panel de curaduría es una **SPA React + Vite + TypeScript** que se compila dentro de la imagen del **gateway** y se sirve como estático bajo `/curaduria/`, autenticada con el mismo JWT del `auth-service`. Los formularios de `atributos_especificos` se generan en runtime desde `GET /api/v1/schemas`, no desde una copia de los schemas en el front | Server-side rendering (Next.js) / plantillas HTML servidas por `especies-api` / duplicar los JSON Schemas en el front | El panel es una herramienta interna para una decena de personas: no necesita SEO ni SSR, y un bundle estático no añade ningún proceso que mantener, monitorear ni escalar —cabe en el contenedor de Nginx que ya existe. React es además el stack que ya se usa en el móvil, así que no abre un frente nuevo de aprendizaje. Servir los schemas desde la API en vez de copiarlos evita el fallo silencioso clásico: que el formulario acepte un campo que el validador del servidor rechaza. Consecuencia asumida: `VITE_S3_PUBLIC_BASE` se hornea en tiempo de build (un bundle estático no lee configuración en runtime), así que cambiar la URL pública del object storage obliga a reconstruir la imagen del gateway. |

Cualquier cambio futuro a estas decisiones debe quedar como una entrada nueva con fecha y justificación, no editar la anterior.

---

## 11. Riesgos y mitigaciones

- **Curva del NDK Camera2**: API verbosa en C. Mitigación: empezar con un MVP que abra/capture, sumar controles manuales iterativamente.
- **Validación JSON Schema en C++**: librerías como `nlohmann/json-schema-validator`. Verificar licencia y rendimiento.
- **Backup y pérdida de fotos en VPS**: si el disco muere, perdemos todo. Mitigación: `mc mirror` nocturno a B2 + alerta si falla.
- **Costo Google Sign-In en producción**: gratis bajo cuota razonable, pero requiere consola de Google Cloud configurada (OAuth client IDs para Android y backend).
- **Comestibilidad de hongos**: si publicamos información incorrecta sobre toxicidad, hay riesgo legal/sanitario. Mitigación: campo `revisado_por` obligatorio en Fungi para que sea visible al público, banner explícito en la app: "consulte un experto antes de consumir".

---

## 12. Preguntas abiertas

Cosas que no necesito resolver hoy pero hay que pensar antes de la fase correspondiente:

- ¿Sonidos para Animalia (aves/anfibios)? Si sí, hay que extender el bucket y el modelo.
- ¿Mapa con avistamientos en la app? Si sí, qué proveedor (Mapbox cuesta, OpenStreetMap es gratis pero más limitado).
- ¿Soporte i18n? Idea: español (default), inglés, mapudungun cuando haya colaboradores.
- ¿Notificaciones push? FCM es lo natural si ya usamos Google.
- ¿Open-sourcear el dataset? Tiene valor académico; decidir licencia (CC-BY-SA podría encajar).
- **Red social completa** (objetivo futuro, no planificado aún): que el perfil evolucione a
  red social — seguir a otros usuarios, ver sus encuentros públicos, likes/comentarios. La
  Fase 8b (perfil con avatar, encuentros con visibilidad privado/público, compartir externo)
  deja la base de datos y la UI listas para esa transición, pero no la implementa.
- **Reconocimiento de especies por IA local** (objetivo futuro, no planificado aún): el
  usuario sube una foto y un modelo corriendo en nuestro propio backend (no una API cloud de
  terceros) identifica la especie o confirma la que el usuario sugiere. Análisis de
  viabilidad:
  - El VPS de Fase 7 (mínimo 4 vCPU/8GB RAM, sin GPU) alcanza para **inferencia** de un
    modelo liviano (MobileNetV3/EfficientNet-Lite cuantizado, runtime ONNX/TFLite, CPU-only,
    latencia esperada de cientos de ms por imagen) pero no para **entrenar** uno desde cero
    en tiempos razonables.
  - Enfoque realista: entrenar el modelo fuera de la VPS (máquina propia o rentada
    puntualmente con GPU, ej. una RTX de gama media) y desplegar solo el modelo ya entrenado
    (inferencia CPU en el VPS, o incluso on-device en la app, coherente con el ADR de
    offline-first).
  - Requisito bloqueante real, no paralelo: un dataset de fotos etiquetadas por especie. Hoy
    es prácticamente inexistente (Fase 8 de contenido curado sigue pendiente). Las fotos que
    los usuarios suban en sus encuentros privados (Fase 8b) generan ese dataset
    orgánicamente con el tiempo.
  - Alternativa de respaldo si entrenar un modelo propio no resulta viable: APIs cloud de
    visión (Google Vision AutoML, AWS Rekognition Custom Labels) — más rápidas de
    bootstrapear pero dejan de ser "IA local" y agregan costo por llamada y dependencia de
    terceros.
  - Conclusión: viable a mediano plazo, condicionado a (1) acumular dataset vía encuentros de
    usuarios, (2) acceso puntual a GPU para entrenar, (3) decidir inferencia on-device vs
    servidor una vez exista el modelo.

---

## 13. Cómo usar este documento

- Cualquier PR que toque arquitectura debe referenciarlo y actualizarlo en el mismo PR.
- Si una decisión cambia, **no se borra**, se añade una entrada nueva en §10 con la fecha.
- Si aparece una nueva pregunta abierta, se añade en §12.
- El plan es vivo, no inmutable.
