# Estructura del Proyecto Flora Chiloé

```
chiloe-flora-microservices/
├── services/
│   ├── especies-api/                 # Tu API actual en C++
│   │   ├── src/
│   │   ├── include/
│   │   ├── CMakeLists.txt
│   │   ├── Dockerfile
│   │   └── Dockerfile.dev         # Para desarrollo con hot reload
│   │
│   ├── auth-service/              # Nuevo servicio en Go
│   │   ├── cmd/
│   │   ├── internal/
│   │   ├── pkg/
│   │   ├── go.mod
│   │   ├── Dockerfile
│   │   └── Dockerfile.dev
│   │
│   └── gateway/                   # API Gateway
│       ├── nginx.conf
│       └── Dockerfile
│
├── infrastructure/
│   ├── terraform/                 # Infraestructura AWS
│   │   ├── modules/
│   │   ├── environments/
│   │   │   ├── dev/
│   │   │   └── prod/
│   │   └── main.tf
│   │
│   ├── kubernetes/                # Manifiestos K8s
│   │   ├── base/
│   │   ├── dev/
│   │   └── prod/
│   │
│   └── docker/                    # Docker Compose para desarrollo
│       ├── docker-compose.yml
│       └── docker-compose.dev.yml
│
├── .github/workflows/             # CI/CD Pipeline
│   ├── especies-api.yml
│   ├── auth-service.yml
│   └── deploy.yml
│
│
└── README.md
```

## Desarrollo Local con Hot Reload

### Opción 1: Docker Compose (Recomendada para desarrollo)
Cada servicio tendrá su propio contenedor con volúmenes montados para hot reload.

### Opción 2: Desarrollo Nativo + Docker para Dependencias
Servicios corriendo nativamente con PostgreSQL en Docker.

## Despliegue en AWS

### Infraestructura Recomendada:
1. **EKS** (Kubernetes administrado) - Más escalable que EC2 manual
2. **ALB** (Application Load Balancer) - Para enrutamiento
3. **RDS PostgreSQL** - Base de datos administrada
4. **ECR** - Registro de contenedores
5. **CloudWatch** - Monitoreo y logs

### Pipeline CI/CD:
1. **GitHub Actions** - Para CI/CD
2. **Terraform** - Para infraestructura
3. **Helm** - Para despliegues en Kubernetes