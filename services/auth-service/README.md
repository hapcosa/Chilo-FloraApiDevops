# Auth Service - Flora Chiloé

Servicio de autenticación y autorización para el ecosistema de microservicios Flora Chiloé. Desarrollado en Go con Gin y PostgreSQL.

## Características

- **Autenticación JWT**: Tokens seguros con refresh automático
- **OAuth 2.0**: Integración con Google y GitHub
- **Roles y Permisos**: Sistema de autorización granular
- **Rate Limiting**: Protección contra ataques de fuerza bruta
- **Health Checks**: Endpoints para monitoreo y orquestación
- **Hot Reload**: Desarrollo ágil con recarga automática
- **Métricas**: Instrumentación para Prometheus

## 📁 Estructura del Proyecto

```
auth-service/
├── cmd/
│   └── main.go                    # Punto de entrada de la aplicación
├── internal/
│   ├── api/
│   │   └── handlers/
│   │       ├── auth.go            # Handlers de autenticación
│   │       ├── health.go          # Health checks
│   │       └── routes.go          # Configuración de rutas
│   ├── config/
│   │   └── config.go              # Configuración de la aplicación
│   ├── database/
│   │   └── database.go            # Conexión y migraciones
│   ├── middleware/
│   │   └── middleware.go          # Middlewares personalizados
│   ├── models/
│   │   └── user.go                # Modelos de datos
│   └── services/
│       ├── auth.go                # Lógica de autenticación
│       └── oauth.go               # Servicios OAuth
├── .air.toml                      # Configuración para hot reload
├── .env                           # Variables de entorno (desarrollo)
├── Dockerfile                     # Imagen para producción
├── Dockerfile.dev.dock            # Imagen para desarrollo
├── estructura-auth-service.md     # Documentación de arquitectura
├── estructura.md                  # Estructura general del proyecto
├── go.mod                         # Dependencias de Go
├── go.sum                         # Checksums de dependencias
└── README.md                      # Este archivo
```

## Tecnologías Utilizadas

- **Go 1.21+**: Lenguaje principal
- **Gin**: Framework web HTTP
- **GORM**: ORM para Go
- **PostgreSQL**: Base de datos principal
- **JWT-Go**: Manejo de tokens JWT
- **OAuth2**: Autenticación con terceros
- **Air**: Hot reload para desarrollo
- **Docker**: Containerización

## 🚦 Endpoints de la API

### Salud del Servicio
```http
GET /health          # Estado general del servicio
GET /ready           # Readiness probe (K8s)
GET /alive           # Liveness probe (K8s)
GET /metrics         # Métricas de Prometheus
```

### Autenticación Pública
```http
POST /api/v1/auth/register           # Registro de usuario
POST /api/v1/auth/login              # Inicio de sesión
POST /api/v1/auth/refresh            # Renovar token JWT
GET  /api/v1/auth/verify             # Verificar token (nginx)
GET  /api/v1/auth/whoami             # Info del usuario actual
```

### OAuth 2.0
```http
GET /api/v1/auth/google              # Iniciar OAuth con Google
GET /api/v1/auth/github              # Iniciar OAuth con GitHub
GET /api/v1/auth/callback/google     # Callback de Google
GET /api/v1/auth/callback/github     # Callback de GitHub
```

### Rutas Protegidas (Requieren JWT)
```http
GET  /api/v1/auth/me                 # Perfil del usuario
PUT  /api/v1/auth/me                 # Actualizar perfil
POST /api/v1/auth/change-password    # Cambiar contraseña
POST /api/v1/auth/logout             # Cerrar sesión
```

### Administración (Solo Admin)
```http
GET    /api/v1/auth/admin/users         # Listar usuarios
GET    /api/v1/auth/admin/users/:id     # Obtener usuario
PUT    /api/v1/auth/admin/users/:id     # Actualizar usuario
DELETE /api/v1/auth/admin/users/:id     # Eliminar usuario
POST   /api/v1/auth/admin/users/:id/activate    # Activar usuario
POST   /api/v1/auth/admin/users/:id/deactivate  # Desactivar usuario
```

## Configuración

### Variables de Entorno

Crea un archivo `.env` en la raíz del proyecto:

```env
# Servidor
PORT=8080
GIN_MODE=release

# Base de datos
DB_HOST=localhost
DB_PORT=5432
DB_USER=flora_user
DB_PASSWORD=flora_password
DB_NAME=flora_auth
DB_SSL_MODE=disable

# JWT
JWT_SECRET=tu-super-secreto-jwt-key-aqui
JWT_EXPIRATION_HOURS=24
JWT_REFRESH_EXPIRATION_HOURS=168

# OAuth Google
GOOGLE_CLIENT_ID=tu-google-client-id
GOOGLE_CLIENT_SECRET=tu-google-client-secret
GOOGLE_REDIRECT_URL=http://localhost:8080/api/v1/auth/callback/google

# OAuth GitHub
GITHUB_CLIENT_ID=tu-github-client-id
GITHUB_CLIENT_SECRET=tu-github-client-secret
GITHUB_REDIRECT_URL=http://localhost:8080/api/v1/auth/callback/github

# Configuración adicional
ALLOWED_ORIGINS=http://localhost:3000,http://localhost:8080
RATE_LIMIT_REQUESTS=100
RATE_LIMIT_WINDOW=60
```

## 🚀 Desarrollo Local

### Opción 1: Desarrollo Nativo

1. **Instalar dependencias**:
   ```bash
   go mod download
   ```

2. **Configurar base de datos**:
   ```bash
   # Usar Docker para PostgreSQL
   docker run --name flora-postgres \
     -e POSTGRES_USER=flora_user \
     -e POSTGRES_PASSWORD=flora_password \
     -e POSTGRES_DB=flora_auth \
     -p 5432:5432 \
     -d postgres:15
   ```

3. **Ejecutar con hot reload**:
   ```bash
   # Instalar Air (solo primera vez)
   go install github.com/cosmtrek/air@latest
   
   # Ejecutar con hot reload
   air
   ```

4. **Ejecutar sin hot reload**:
   ```bash
   go run cmd/main.go
   ```

### Opción 2: Docker Compose (Desarrollo)

```bash
# Ejecutar en modo desarrollo con hot reload
docker-compose -f docker-compose.dev.yml up

# Ver logs
docker-compose -f docker-compose.dev.yml logs -f auth-service

# Reconstruir imagen
docker-compose -f docker-compose.dev.yml up --build
```


## Testing aun no implementado

```bash
# Ejecutar todos los tests
go test ./...

# Tests con coverage
go test -cover ./...

# Tests verbosos
go test -v ./...

# Test de un paquete específico
go test ./internal/services/
```

## 📊 Monitoreo

### Health Checks

El servicio expone varios endpoints para monitoreo:

- `/health`: Estado general (200 = OK, 503 = Error)
- `/ready`: Readiness probe para Kubernetes
- `/alive`: Liveness probe para Kubernetes
- `/metrics`: Métricas en formato Prometheus

### Ejemplo de respuesta de health:

```json
{
  "status": "ok",
  "timestamp": "2025-08-14T10:30:00Z",
  "version": "1.0.0",
  "database": "connected",
  "uptime": "2h30m15s"
}
```

## 🔒 Seguridad

### Configuración JWT
- Tokens con expiración configurable
- Refresh tokens para renovación automática
- Blacklist de tokens revocados

### Rate Limiting
- Límite de peticiones por IP
- Ventana de tiempo configurable
- Headers informativos en respuestas

### OAuth 2.0
- Estado CSRF para prevenir ataques
- Validación de redirect URLs
- Scopes mínimos necesarios

## Docker

### Desarrollo
```bash
# Imagen con hot reload y tools de desarrollo
FROM golang:1.21-alpine AS dev
RUN go install github.com/cosmtrek/air@latest
WORKDIR /app
COPY . .
EXPOSE 8080
CMD ["air"]
```

### Producción
```bash
# Multi-stage build para imagen optimizada
FROM golang:1.21-alpine AS builder
# ... build steps ...

FROM alpine:latest
# ... production image ...
```

## 🚀 Despliegue

### Variables de Producción

```env
GIN_MODE=release
LOG_LEVEL=info
DB_SSL_MODE=require
CORS_ALLOW_CREDENTIALS=true
RATE_LIMIT_REQUESTS=1000
```


##Contribuir

1. Fork el proyecto
2. Crear feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a branch (`git push origin feature/AmazingFeature`)
5. Abrir Pull Request

##  Licencia no lista aun

Este proyecto está bajo la Licencia MIT. Ver `LICENSE` para más detalles.

## Autores

- **Obrero** - *Desarrollo inicial* - [tu-github](https://github.com/tu-usuario)

