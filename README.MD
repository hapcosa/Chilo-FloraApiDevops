# 🌿 Flora de Chiloé - Arquitectura de Microservicios

Sistema de información sobre la flora endémica de la Isla de Chiloé, implementado con arquitectura de microservicios usando C++, Go, Kubernetes y AWS.

## 🏗️ Arquitectura

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   API Gateway   │────│   Auth Service  │────│   Flora API     │
│    (Nginx)      │    │      (Go)       │    │     (C++)       │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │   PostgreSQL    │
                    │   + Redis       │
                    └─────────────────┘
```

## 🚀 Inicio Rápido

### Prerrequisitos

- Docker & Docker Compose
- Make
- Git

### Configuración Inicial

```bash
# Clonar repositorio
git clone <tu-repositorio>
cd chiloe-flora-microservices

# Configurar entorno de desarrollo
make setup

# Iniciar servicios de desarrollo
make dev
```

## Desarrollo Local

### Comandos Principales

```bash
# Ver todos los comandos disponibles
make help

# Desarrollo con hot reload
make dev

# Ver logs en tiempo real
make dev-logs

# Ejecutar tests
make test

# Verificar salud de servicios
make health-check
```

### URLs de Desarrollo

- **API Gateway**: http://localhost:8080
- **Flora API**: http://localhost:9080
- **Auth Service**: http://localhost:8081
- **PostgreSQL**: localhost:5432

### Hot Reload

Cada servicio está configurado con hot reload:

- **Flora API (C++)**: Detecta cambios en `src/` e `include/` y recompila automáticamente
- **Auth Service (Go)**: Usa Air para recarga automática
- **Gateway**: Configuración dinámica con volúmenes

## 🔧 Servicios

### Flora API (C++ + Pistache)
- **Puerto**: 9080
- **Funcionalidad**: CRUD de familias, géneros y especies
- **Base de datos**: PostgreSQL con libpqxx
- **Hot reload**: Script personalizado con inotify

### Auth Service (Go + Gin)
- **Puerto**: 8081
- **Funcionalidad**: Autenticación JWT, gestión de usuarios
- **Caché**: Redis para sesiones
- **Hot reload**: Air

### API Gateway (Nginx)
- **Puerto**: 8080
- **Funcionalidad**: Enrutamiento, autenticación, rate limiting
- **Load balancing**: Entre instancias de servicios

## 🐳 Docker

### Desarrollo
```bash
# Iniciar todos los servicios
make dev

# Reconstruir contenedores
make dev-build

# Ver logs específicos
docker-compose -f infrastructure/docker/docker-compose.dev.yml logs especies-api
```

### Producción
```bash
# Build de imágenes de producción
make build-all

# Deploy a producción
make aws-deploy
```

## ☸️ Kubernetes

### Minikube (Desarrollo Local)

```bash
# Configurar Minikube
make minikube-setup

# Desplegar en Minikube
make minikube-deploy

# Ver estado
make minikube-status

# URLs de servicios
make minikube-urls

# Dashboard
make minikube-dashboard
```

### AWS EKS (Producción)

```bash
# Planificar infraestructura
make aws-terraform-plan

# Aplicar infraestructura
make aws-terraform-apply

# Desplegar aplicación
make aws-deploy
```

## 🏗️ Infraestructura AWS

### Componentes

- **EKS Cluster**: Kubernetes administrado
- **RDS PostgreSQL**: Base de datos administrada
- **ECR**: Registro de contenedores
- **ALB**: Load balancer para ingress
- **VPC**: Red privada con subnets públicas y privadas

### Terraform

```bash
cd infrastructure/terraform

# Inicializar
terraform init

# Planificar cambios
terraform plan -var="db_password=tu_password_seguro"

# Aplicar
terraform apply -var="db_password=tu_password_seguro"
```

## 🔄 CI/CD

### Pipeline GitHub Actions

El pipeline se ejecuta en:
- **Push a main**: Deploy a producción
- **Push a develop**: Deploy a staging
- **Pull Requests**: Tests y validaciones

### Etapas

1. **Test**: Tests unitarios y de integración
2. **Build**: Construcción de imágenes Docker
3. **Push**: Subida a ECR
4. **Deploy**: Despliegue con Helm a EKS

## 📊 Base de Datos

### Esquema Principal

```sql
-- Familias botánicas
CREATE TABLE familias (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    descripcion TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Géneros
CREATE TABLE generos (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    familia_id INTEGER REFERENCES familias(id),
    descripcion TEXT,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Especies
CREATE TABLE especies (
    id SERIAL PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    nombre_cientifico VARCHAR(150) NOT NULL,
    genero_id INTEGER REFERENCES generos(id),
    descripcion TEXT,
    habitat TEXT,
    estado_conservacion VARCHAR(50),
    created_at TIMESTAMP DEFAULT NOW()
);
```

### Gestión

```bash
# Resetear base de datos
make db-reset

# Conectar a shell
make db-shell

# Backup
make backup-db

# Restaurar
make restore-db BACKUP_FILE=backups/backup_20240101_120000.sql
```

## 🧪 Testing

### Tests Unitarios

```bash
# Todos los tests
make test

# Solo Flora API
make test-especies-api

# Solo Auth Service  
make test-auth-service
```

### Tests de Integración

```bash
# Simular pipeline completo
make ci-local
```

## 📈 Monitoreo

### Logs

```bash
# Logs en tiempo real
make logs

# Logs específicos
make dev-logs

# En Kubernetes
kubectl logs -f deployment/especies-api -n chiloe-flora
```

### Health Checks

Todos los servicios exponen `/health`:

```bash
# Verificar salud
make health-check

# Manualmente
curl http://localhost:8080/health
curl http://localhost:9080/health  
curl http://localhost:8081/health
```

## 🔒 Seguridad

### Autenticación

- **JWT Tokens**: Para autenticación de usuarios
- **API Keys**: Para comunicación entre servicios
- **Rate Limiting**: En API Gateway

### Variables de Entorno

```bash
# Desarrollo (.env)
DB_PASSWORD=development_password
JWT_SECRET=dev_jwt_secret

# Producción (Kubernetes Secrets)
kubectl create secret generic app-secrets \
  --from-literal=DB_PASSWORD=secure_production_password \
  --from-literal=JWT_SECRET=super_secure_jwt_secret
```

## 📚 API Endpoints

### Auth Service (`:8081`)

```
POST   /api/v1/auth/register     - Registro de usuario
POST   /api/v1/auth/login        - Login
POST   /api/v1/auth/refresh      - Renovar token
GET    /api/v1/auth/profile      - Perfil de usuario (protegido)
PUT    /api/v1/auth/profile      - Actualizar perfil (protegido)
POST   /api/v1/auth/logout       - Logout (protegido)
```

### Flora API (`:9080`)

```
GET    /api/v1/familias          - Listar familias
POST   /api/v1/familias          - Crear familia (protegido)
GET    /api/v1/familias/:id      - Ver familia
PUT    /api/v1/familias/:id      - Actualizar familia (protegido)
DELETE /api/v1/familias/:id      - Eliminar familia (protegido)

GET    /api/v1/generos           - Listar géneros
POST   /api/v1/generos           - Crear género (protegido)
GET    /api/v1/generos/:id       - Ver género
PUT    /api/v1/generos/:id       - Actualizar género (protegido)
DELETE /api/v1/generos/:id       - Eliminar género (protegido)

GET    /api/v1/especies          - Listar especies
POST   /api/v1/especies          - Crear especie (protegido)
GET    /api/v1/especies/:id      - Ver especie
PUT    /api/v1/especies/:id      - Actualizar especie (protegido)
DELETE /api/v1/especies/:id      - Eliminar especie (protegido)
```

## 🐛 Debugging

### Logs de Desarrollo

```bash
# Contenedor específico
docker-compose -f infrastructure/docker/docker-compose.dev.yml logs -f especies-api

# Shell en contenedor
make shell-especies-api
make shell-auth-service
make shell-postgres
```

### Debugging C++

```bash
# Compilar en modo debug
cd services/especies-api
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Con GDB en contenedor
docker-compose -f infrastructure/docker/docker-compose.dev.yml exec especies-api gdb ./build/chiloe_especies_api
```

## 📝 Contribuir

1. Fork del repositorio
2. Crear rama feature: `git checkout -b feature/nueva-funcionalidad`
3. Commit: `git commit -am 'Agregar nueva funcionalidad'`
4. Push: `git push origin feature/nueva-funcionalidad`
5. Pull Request

### Estándares de Código

```bash
# Formatear código
make format

# Análisis estático
make lint
```

## 📄 Licencia

Este proyecto está bajo la Licencia MIT. Ver `LICENSE` para más detalles.

## 🆘 Solución de Problemas

### Problemas Comunes

**Error: Port already in use**
```bash
make dev-stop
make clean
make dev
```

**Error: No space left on device**
```bash
make clean
docker system prune -a -f
```

**Error: Minikube not starting**
```bash
minikube delete
minikube start --driver=docker
```

### Contacto

- **Desarrollador**: Hernan
- **Email**: hapcosa@gmail.com
- **Issues**: GitHub Issues del repositorio