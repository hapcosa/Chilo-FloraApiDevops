# Dockerfile de desarrollo para Auth Service
FROM golang:1.21-alpine AS base

# Instalar dependencias del sistema
RUN apk add --no-cache git ca-certificates tzdata

# Configurar timezone
ENV TZ=America/Santiago
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Instalar Air para hot reload
RUN go install github.com/cosmtrek/air@latest

# Crear directorio de trabajo
WORKDIR /app

# Copiar go.mod y go.sum
COPY go.mod go.sum ./

# Descargar dependencias
RUN go mod download

# Crear usuario no-root
RUN adduser -D -s /bin/sh appuser
RUN chown -R appuser:appuser /app

# Cambiar a usuario no-root
USER appuser

# Configurar variables de entorno para desarrollo
ENV GIN_MODE=debug
ENV API_PORT=8081
ENV ENVIRONMENT=development

# Exponer puerto
EXPOSE 8081

# Comando por defecto con Air para hot reload
CMD ["air", "-c", ".air.toml"]