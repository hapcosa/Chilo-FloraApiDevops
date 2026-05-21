#!/bin/bash
# scripts/dev.sh - Script para levantar entorno de desarrollo

set -e

echo "🚀 Iniciando entorno de desarrollo Chiloé Flora..."

# Verificar que Docker esté corriendo
if ! docker info > /dev/null 2>&1; then
    echo "❌ Docker no está corriendo. Por favor inicia Docker."
    exit 1
fi

# Limpiar contenedores anteriores si existen
echo "🧹 Limpiando contenedores anteriores..."
docker-compose --env-file .env -f docker-compose.dev.yml down --remove-orphans

# Opcional: limpiar volúmenes (usar con cuidado)
if [ "$1" = "clean" ]; then
    echo "🗑️  Limpiando volúmenes..."
    docker-compose --env-file .env -f docker-compose.dev.yml down -v
    docker volume prune -f
fi

# Construir e iniciar servicios
echo "🏗️  Construyendo y iniciando servicios..."
docker-compose --env-file .env -f docker-compose.dev.yml up --build -d postgres redis

# Esperar a que la base de datos esté lista
echo "⏳ Esperando que PostgreSQL esté listo..."
docker-compose --env-file .env -f docker-compose.dev.yml exec postgres pg_isready -U dev_user -d chiloe_flora_dev

# Iniciar Flora API con logs en tiempo real
echo "🌿 Iniciando Flora API con hot reload..."
docker-compose --env-file .env -f docker-compose.dev.yml up --build especies-api

echo "✅ Entorno de desarrollo iniciado!"
echo "📡 Flora API: http://localhost:9081"
echo "🐘 PostgreSQL: localhost:5433"
echo "🔴 Redis: localhost:6379"
echo ""
echo "Para detener: Ctrl+C"
echo "Para limpiar todo: ./scripts/dev.sh clean"