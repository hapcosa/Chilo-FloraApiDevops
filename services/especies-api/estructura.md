# Estructura del Proyecto - Chiloé Flora API

```
especies-api/
├── 📁 config/                      # Configuraciones del proyecto
├── 📁 include/                     # Archivos de cabecera (.hpp)
│   ├── 📁 controllers/             # Controladores HTTP
│   │   ├── 📄 especie_controller.hpp
│   │   ├── 📄 familia_controller.hpp
│   │   └── 📄 genero_controller.hpp
│   ├── 📁 models/                  # Modelos de datos
│   │   ├── 📄 especie.hpp
│   │   ├── 📄 familia.hpp
│   │   ├── 📄 genero.hpp
│   │   └── 📄 imagen.hpp
│   ├── 📁 repository/              # Interfaces y repositorios
│   │   ├── 📄 especie_repository.hpp
│   │   ├── 📄 familia_repository.hpp
│   │   ├── 📄 genero_repository.hpp
│   │   ├── 📄 postgres_familia_repository.hpp
│   │   ├── 📄 postgres_genero_repository.hpp
│   │   └── 📄 postgresql_especie_repository.hpp
│   ├── 📁 services/                # Lógica de negocio
│   │   ├── 📄 especie_service.hpp
│   │   ├── 📄 familia_service.hpp
│   │   └── 📄 genero_service.hpp
│   └── 📁 utils/                   # Utilidades y helpers
│       ├── 📄 config.hpp
│       ├── 📄 constants.hpp
│       ├── 📄 database.hpp
│       └── 📄 dotenv.hpp
├── 📁 scripts/                     # Scripts de automatización
├── 📁 src/                         # Código fuente (.cpp)
│   ├── 📁 controllers/             # Implementación de controladores
│   │   ├── 📄 especie_controller.cpp
│   │   ├── 📄 familia_controller.cpp
│   │   └── 📄 genero_controller.cpp
│   ├── 📁 models/                  # Implementación de modelos
│   │   ├── 📄 especie.cpp
│   │   ├── 📄 familia.cpp
│   │   ├── 📄 genero.cpp
│   │   └── 📄 imagen.cpp
│   ├── 📁 repository/              # Implementación de repositorios
│   │   ├── 📄 postgres_familia_repository.cpp
│   │   ├── 📄 postgres_genero_repository.cpp
│   │   └── 📄 postgresql_especie_repository.cpp
│   ├── 📁 services/                # Implementación de servicios
│   │   ├── 📄 especie_service.cpp
│   │   ├── 📄 familia_service.cpp
│   │   └── 📄 genero_service.cpp
│   ├── 📁 utils/                   # Implementación de utilidades
│   │   ├── 📄 config.cpp
│   │   ├── 📄 database.cpp
│   │   └── 📄 dotenv.cpp
│   └── 📄 main.cpp                 # Punto de entrada principal
├── 📄 .env                         # Variables de entorno (no versionado)
├── 📄 .env.example                 # Plantilla de variables de entorno
├── 📄 CMakeLists.txt               # Configuración de CMake
├── 📄 Dockerfile.dev.dock          # Docker para desarrollo
├── 📄 dev.sh                       # Script de desarrollo
├── 📄 docker-compose.yml           # Composición de servicios (producción)
├── 📄 docker-compose.dev.yml       # Composición de servicios (desarrollo)
└── 📄 README.md                    # Documentación principal
```

## Arquitectura del Proyecto

### 🏗️ Patrón Arquitectónico
El proyecto sigue una **Arquitectura por Capas (Layered Architecture)** con separación clara de responsabilidades:

```
┌─────────────────────────────────┐
│          Controllers            │ ← Capa de Presentación
│     (HTTP Request Handlers)     │
├─────────────────────────────────┤
│           Services              │ ← Capa de Lógica de Negocio
│       (Business Logic)         │
├─────────────────────────────────┤
│         Repositories            │ ← Capa de Acceso a Datos
│      (Data Access Layer)       │
├─────────────────────────────────┤
│            Models               │ ← Capa de Datos
│        (Data Entities)         │
├─────────────────────────────────┤
│          Database               │ ← Persistencia
│        (PostgreSQL)            │
└─────────────────────────────────┘
```

### 📋 Descripción de Capas

#### 1. **Controllers** (`src/controllers/`, `include/controllers/`)
- **Responsabilidad**: Manejar peticiones HTTP y respuestas
- **Tecnología**: Pistache HTTP Framework
- **Funciones**:
    - Validación de parámetros de entrada
    - Manejo de errores HTTP
    - Serialización/deserialización JSON
    - Gestión de archivos de imagen

#### 2. **Services** (`src/services/`, `include/services/`)
- **Responsabilidad**: Lógica de negocio y orquestación
- **Funciones**:
    - Validación de reglas de negocio
    - Orquestación entre repositorios
    - Procesamiento de imágenes
    - Transformación de datos

#### 3. **Repositories** (`src/repository/`, `include/repository/`)
- **Responsabilidad**: Acceso y persistencia de datos
- **Patrón**: Repository Pattern
- **Tecnología**: PostgreSQL con libpqxx
- **Funciones**:
    - Operaciones CRUD
    - Consultas especializadas
    - Gestión de transacciones

#### 4. **Models** (`src/models/`, `include/models/`)
- **Responsabilidad**: Definición de entidades de datos
- **Funciones**:
    - Serialización JSON
    - Validación de datos
    - Mapeo objeto-relacional

#### 5. **Utils** (`src/utils/`, `include/utils/`)
- **Responsabilidad**: Utilidades transversales
- **Componentes**:
    - Gestión de configuración
    - Conexión a base de datos
    - Carga de variables de entorno
    - Constantes globales

## 🗄️ Entidades Principales

### **Familia** → **Género** → **Especie**
```
Familia (1:N) → Género (1:N) → Especie
   ↓              ↓              ↓
Imágenes      Imágenes      Imágenes
```

### **Estructura de Datos**:
- **Familia**: Agrupación taxonómica superior
- **Género**: Clasificación intermedia
- **Especie**: Unidad básica taxonómica
- **Imagen**: Recursos multimedia asociados

## 🔧 Tecnologías Utilizadas

| Componente | Tecnología | Versión |
|------------|------------|---------|
| **Framework HTTP** | Pistache | Latest |
| **Base de Datos** | PostgreSQL | 15+ |
| **ORM/Driver** | libpqxx | 7.8.1+ |
| **JSON** | nlohmann/json | 3.11.3 |
| **Build System** | CMake | 3.10+ |
| **Containerización** | Docker | Latest |
| **Orquestación** | Docker Compose | Latest |
| **Lenguaje** | C++17 | ISO/IEC 14882:2017 |

## 🚀 Flujo de Datos

```
HTTP Request → Controller → Service → Repository → Database
     ↓             ↓          ↓          ↓          ↓
JSON Response ← Controller ← Service ← Repository ← Database
```

### Ejemplo de Flujo Completo:
1. **Cliente** envía `GET /api/especies/1`
2. **Router** dirige a `EspecieController::getById`
3. **Controller** valida parámetros y llama a `EspecieService::getEspecieById`
4. **Service** aplica lógica de negocio y llama a `EspecieRepository::findById`
5. **Repository** ejecuta consulta SQL en PostgreSQL
6. **Respuesta** se serializa como JSON y se envía al cliente

## 📁 Convenciones de Nombrado

- **Clases**: PascalCase (`EspecieController`)
- **Métodos**: camelCase (`getEspecieById`)
- **Archivos**: snake_case (`especie_controller.cpp`)
- **Variables**: camelCase (`especieId`)
- **Constantes**: UPPER_SNAKE_CASE (`MAX_FILE_SIZE`)
- **Endpoints**: kebab-case (`/api/especies/search/nombre-cientifico`)

## 🔒 Seguridad y Validación

- **Validación de entrada**: Sanitización de parámetros HTTP
- **Prevención Path Traversal**: Validación de nombres de archivos
- **Límites de tamaño**: Archivos de imagen máximo 10MB
- **Manejo de errores**: Respuestas HTTP estándar
- **Logging**: Registro de operaciones críticas