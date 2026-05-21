# Makefile para el proyecto Flora Chiloé

.PHONY: help dev dev-down build clean logs test

# Variables
COMPOSE_FILE := infrastructure/docker/docker-compose.dev.yml
COMPOSE := docker-compose -f $(COMPOSE_FILE)

help: ## Mostrar esta ayuda
	@echo "Comandos disponibles:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'

dev: ## Iniciar entorno de desarrollo
	@echo "🚀 Iniciando entorno de desarrollo..."
	@cd infrastructure/docker && $(COMPOSE) up -d

dev-full: ## Iniciar entorno completo con herramientas
	@echo "🚀 Iniciando entorno completo de desarrollo..."
	@cd infrastructure/docker && $(COMPOSE) --profile tools --profile monitoring up -d

dev-down: ## Detener entorno de desarrollo
	@echo "🛑 Deteniendo entorno de desarrollo..."
	@cd infrastructure/docker && $(COMPOSE) down

dev-down-volumes: ## Detener y eliminar volúmenes
	@echo "🗑️ Deteniendo y eliminando volúmenes..."
	@cd infrastructure/docker && $(COMPOSE) down -v

build: ## Construir todas las imágenes
	@echo "🔨 Construyendo imágenes..."
	@cd infrastructure/docker && $(COMPOSE) build

build-no-cache: ## Construir sin caché
	@echo "🔨 Construyendo imágenes sin caché..."
	@cd infrastructure/docker && $(COMPOSE) build --no-cache

logs: ## Ver logs de todos los servicios
	@cd infrastructure/docker && $(COMPOSE) logs -f

logs-especies: ## Ver logs del servicio Especies API
	@cd infrastructure/docker && $(COMPOSE) logs -f especies-api

logs-auth: ## Ver logs del servicio Auth
	@cd infrastructure/docker && $(COMPOSE) logs -f auth-service

logs-gateway: ## Ver logs del gateway
	@cd infrastructure/docker && $(COMPOSE) logs -f gateway

restart: ## Reiniciar todos los servicios
	@echo "🔄 Reiniciando servicios..."
	@cd infrastructure/docker && $(COMPOSE) restart

restart-especies: ## Reiniciar solo Especies API
	@echo "🔄 Reiniciando Especies API..."
	@cd infrastructure/docker && $(COMPOSE) restart especies-api

restart-auth: ## Reiniciar solo Auth Service
	@echo "🔄 Reiniciando Auth Service..."
	@cd infrastructure/docker && $(COMPOSE) restart auth-service

ps: ## Ver estado de los servicios
	@cd infrastructure/docker && $(COMPOSE) ps

exec-especies: ## Entrar al contenedor Especies API
	@cd infrastructure/docker && $(COMPOSE) exec especies-api sh

exec-auth: ## Entrar al contenedor Auth Service
	@cd infrastructure/docker && $(COMPOSE) exec auth-service sh

exec-db: ## Entrar a PostgreSQL
	@cd infrastructure/docker && $(COMPOSE) exec postgres psql -U dev_user -d chiloe_flora_dev

clean: ## Limpiar contenedores, imágenes y volúmenes no utilizados
	@echo "🧹 Limpiando Docker..."
	docker system prune -f
	docker volume prune -f

db-reset: ## Resetear base de datos (eliminar volumen y recrear)
	@echo "Reseteando base de datos..."
	@cd infrastructure/docker && $(COMPOSE) down
	docker volume rm docker_postgres_dev_data 2>/dev/null || true
	@cd infrastructure/docker && $(COMPOSE) up -d postgres
	@echo "⏳ Esperando que la base de datos esté lista..."
	@sleep 10
	@cd infrastructure/docker && $(COMPOSE) up -d

# Comandos de desarrollo Go
go-mod-tidy: ## Limpiar módulos Go del auth-service
	@echo " Limpiando módulos Go..."
	@cd services/auth-service && go mod tidy

go-test: ## Ejecutar tests del auth-service
	@echo " Ejecutando tests..."
	@cd services/auth-service && go test ./...

go-lint: ## Lint del código Go
	@echo "Ejecutando lint..."
	@cd services/auth-service && golangci-lint run

# Comandos de C++
cpp-build: ## Compilar Especies API localmente
	@echo "Compilando Especies API..."
	@cd services/especies-api && mkdir -p build && cd build && cmake .. && make -j4

cpp-clean: ## Limpiar build de C++
	@echo "Limpiando build de C++..."
	@cd services/especies-api && rm -rf build

# Comandos útiles
api-test: ## Probar endpoints básicos
	@echo "Probando endpoints básicos..."
	@echo "Health checks:"
	@curl -s http://localhost:8080/health | jq . || echo "Gateway no disponible"
	@curl -s http://localhost:8081/health | jq . || echo "Auth service no disponible"
	@curl -s http://localhost:9081/health | jq . || echo "Especies API no disponible"

setup: ## Configuración inicial del proyecto
	@echo "🏗️ Configuración inicial..."
	@echo "Creando archivo .env si no existe..."
	@test -f .env || cp .env.example .env
	@echo "Creando archivo .env para auth-service..."
	@cd services/auth-service && test -f .env || cp .env.example .env
	@echo "✅ Configuración completa. Edita los archivos .env según necesites."

info: ## Mostrar información del proyecto
	@echo "📋 Información del proyecto Flora Chiloé"
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo "🌐 URLs de desarrollo:"
	@echo "   Gateway:      http://localhost:8080"
	@echo "   Auth Service: http://localhost:8081"
	@echo "   Especies API:    http://localhost:9081"
	@echo "   PgAdmin:      http://localhost:8889"
	@echo ""
	@echo "🔧 Herramientas (con profile tools):"
	@echo "   Prometheus:   http://localhost:9090"
	@echo "   Grafana:      http://localhost:3000"
	@echo ""
	@echo "📝 Comandos útiles:"
	@echo "   make dev           - Iniciar desarrollo"
	@echo "   make logs          - Ver logs"
	@echo "   make api-test      - Probar APIs"