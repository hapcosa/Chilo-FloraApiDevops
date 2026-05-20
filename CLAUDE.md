# CLAUDE.md — Guía para asistentes de IA en este repo

Documento de orientación para cualquier agente (Claude Code u otro) que trabaje en este proyecto. Si lo que vas a hacer no encaja con lo descrito aquí, **pregunta antes de actuar**.

---

## Qué es este proyecto

Sistema de divulgación científica sobre la **biodiversidad de Chiloé** (los cinco reinos). Tiene dos partes:

1. **Backend microservicios** (este repo): catálogo CRUD multi-reino, autenticación, gateway, fotos en object storage.
2. **App móvil Android** (submódulo `mobile/`, React Native + módulo nativo C++): biblioteca, login Google/local, captura de fotos con NDK Camera2, soporte offline.

El plan completo y vivo está en [docs/PLAN_MAESTRO.md](docs/PLAN_MAESTRO.md). **Léelo antes de proponer cambios estructurales.**

---

## Stack

| Componente | Tecnología | Carpeta |
|------------|------------|---------|
| API de especies | C++17 + Pistache + libpqxx | [services/flora-api/](services/flora-api/) (se renombrará a `especies-api`) |
| Auth | Go + Gin + JWT | [services/auth-service/](services/auth-service/) |
| Gateway | Nginx | [services/gateway/](services/gateway/) |
| BD | PostgreSQL | gestionado por compose / K8s |
| Cache/sesión | Redis | gestionado por compose / K8s |
| Object storage | MinIO (dev/k3s) / S3 (cloud) | en infra |
| Orquestación local | Docker Compose / minikube | [infrastructure/](infrastructure/) |
| Producción | k3s en VPS Linux | [infrastructure/kubernetes/](infrastructure/kubernetes/) |
| App móvil | React Native (bare CLI) + NDK Camera2 (C++) | `mobile/` (submódulo, pendiente de crear) |
| CI/CD | GitHub Actions | [.github/workflows/](.github/workflows/) |

---

## Reglas de trabajo (pipeline)

**Innegociable**. Cualquier cambio sigue este ciclo:

1. `git checkout -b <tipo>/<descripcion-corta>` (`feat/`, `fix/`, `refactor/`, `docs/`, `chore/`).
2. Cambios y commits con mensajes claros.
3. `git push -u origin <rama>` — GitHub Actions corre tests.
4. Abrir Pull Request contra `master`.
5. Esperar que **todos los checks pasen** y que haya revisión.
6. Merge a `master` (squash o merge commit, consistente con lo que ya haya).
7. El workflow de deploy se dispara automáticamente.

**No** hagas push directo a `master`. **No** ejecutes `git push --force` ni `git reset --hard` sin permiso explícito. **No** uses `--no-verify` para saltarte hooks.

---

## Convenciones de código

- **C++**: estilo del proyecto existente (mira [services/flora-api/src/](services/flora-api/src/)). Indentación 4 espacios, headers en `include/`, implementación en `src/`. Tests con gtest.
- **Go**: `gofmt` y `golangci-lint`. Estructura ya establecida (`cmd/`, `internal/api`, `internal/services`, etc.).
- **SQL**: migraciones numeradas en `services/<servicio>/migrations/`. Una migración por PR cuando sea posible.
- **React Native** (cuando exista): TypeScript estricto, ESLint, Prettier. Componentes funcionales con hooks.
- **Comentarios**: solo donde el *por qué* no es evidente. No documentes lo obvio.
- **Sin features especulativas**: no añadas abstracciones para necesidades hipotéticas.

---

## Modelo de datos multi-reino

Detalle completo en [docs/PLAN_MAESTRO.md §3](docs/PLAN_MAESTRO.md). Resumen mínimo:

- Tabla `especies` con columnas comunes (nombre, científico, descripción, hábitat, fotos) + **columna `atributos_especificos JSONB`** validada por JSON Schema según `reino`.
- Enum `reino_enum`: `animalia | plantae | fungi | protista | monera`.
- Fotos: solo guardamos **claves de object storage**, nunca bytes en la BD.

Cuando añadas un campo nuevo:
- Si aplica a todos los reinos → columna nueva en `especies`.
- Si aplica a uno o algunos → propiedad nueva en el JSON Schema de ese reino, no columna.

---

## Fotos

- **Nunca** subir fotos como `multipart/form-data` a la API en producción.
- Flujo: cliente pide presigned URL → sube directo a MinIO/S3 → notifica la key a la API.
- Strip de EXIF sensible (GPS, serial del dispositivo) salvo que el usuario opte por compartirlo.

---

## Autenticación

- JWT propio emitido por `auth-service`, validado por el gateway con `auth_request`.
- Login Google: el cliente obtiene `idToken` con el SDK de Google Sign-In y lo envía a `POST /api/v1/auth/google`. El backend lo verifica contra Google y emite el JWT propio.
- **No** introducir Firebase Auth, Auth0 u otro proveedor sin discutirlo en `PLAN_MAESTRO.md`.

---

## Comandos útiles (heredados del Makefile)

```bash
make dev              # Levantar todos los servicios con hot reload
make dev-logs         # Ver logs
make test             # Tests de todos los servicios
make health-check     # Verificar /health de cada servicio
make minikube-deploy  # Deploy a minikube
make db-shell         # Shell a Postgres
```

URLs locales:
- Gateway: http://localhost:8080
- Flora/especies API: http://localhost:9080
- Auth: http://localhost:8081

---

## Qué hacer cuando algo no está claro

1. **Lee primero**: [docs/PLAN_MAESTRO.md](docs/PLAN_MAESTRO.md) y la sección relevante del [README.MD](README.MD).
2. **Pregunta**: si el plan no cubre lo que necesitas, pregunta al humano antes de improvisar.
3. **Si cambias arquitectura**, actualiza `docs/PLAN_MAESTRO.md` en el **mismo PR** y añade entrada en §10 (decisiones).

---

## Qué NO hacer

- No renombrar carpetas o servicios sin un PR explícito de renombrado.
- No introducir dependencias nuevas sin justificarlas en el PR.
- No tocar `master` directamente.
- No commitear secretos (`.env` reales, keys de Google, kubeconfig).
- No bypassear hooks de pre-commit ni de CI.
- No añadir backwards-compat hacks "por si acaso".
- No diseñar para reinos hipotéticos fuera de los 5 acordados.

---

## Idioma

Documentación, mensajes de UI y comentarios públicos en **español**. Código (identificadores) y mensajes de error técnicos en **inglés** salvo nombres del dominio biológico (que son universales: `genero`, `familia`, `especie`, `reino`).
