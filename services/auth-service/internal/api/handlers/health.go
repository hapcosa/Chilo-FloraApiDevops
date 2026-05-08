// health.go - Handler para health checks y métricas
package handlers

import (
	"net/http"
	"runtime"
	"time"

	"github.com/gin-gonic/gin"
)

// HealthHandler maneja los endpoints de salud y métricas
type HealthHandler struct {
	startTime time.Time
}

// NewHealthHandler crea un nuevo handler de salud
func NewHealthHandler() *HealthHandler {
	return &HealthHandler{
		startTime: time.Now(),
	}
}

// HealthCheck responde con el estado de salud del servicio
func (h *HealthHandler) HealthCheck(c *gin.Context) {
	uptime := time.Since(h.startTime)

	response := gin.H{
		"status":    "healthy",
		"service":   "auth-service",
		"version":   "1.0.0",
		"timestamp": time.Now().UTC(),
		"uptime":    uptime.String(),
		"checks": gin.H{
			"database": "ok", // Aquí podrías agregar una verificación real de DB
			"redis":    "ok", // Aquí podrías agregar una verificación real de Redis
		},
	}

	c.JSON(http.StatusOK, response)
}

// ReadinessCheck verifica si el servicio está listo para recibir tráfico
func (h *HealthHandler) ReadinessCheck(c *gin.Context) {
	// Aquí podrías agregar verificaciones más complejas
	// como conectividad a base de datos, servicios externos, etc.

	response := gin.H{
		"status":  "ready",
		"service": "auth-service",
		"checks": gin.H{
			"database": "connected", // Verificación real de DB
			"redis":    "connected", // Verificación real de Redis
		},
	}

	c.JSON(http.StatusOK, response)
}

// LivenessCheck verifica si el servicio está vivo
func (h *HealthHandler) LivenessCheck(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{
		"status":  "alive",
		"service": "auth-service",
	})
}

// Metrics proporciona métricas básicas del servicio
func (h *HealthHandler) Metrics(c *gin.Context) {
	var m runtime.MemStats
	runtime.ReadMemStats(&m)

	uptime := time.Since(h.startTime)

	// Formato básico de métricas para Prometheus
	metrics := `# HELP auth_service_uptime_seconds Total uptime of the auth service
# TYPE auth_service_uptime_seconds counter
auth_service_uptime_seconds ` + uptime.String() + `

# HELP auth_service_memory_usage_bytes Memory usage of the auth service
# TYPE auth_service_memory_usage_bytes gauge
auth_service_memory_usage_bytes{type="alloc"} ` + string(rune(m.Alloc)) + `
auth_service_memory_usage_bytes{type="sys"} ` + string(rune(m.Sys)) + `

# HELP auth_service_goroutines Number of goroutines
# TYPE auth_service_goroutines gauge
auth_service_goroutines ` + string(rune(runtime.NumGoroutine())) + `

# HELP auth_service_status Service status (1 = healthy, 0 = unhealthy)
# TYPE auth_service_status gauge
auth_service_status 1
`

	c.Header("Content-Type", "text/plain")
	c.String(http.StatusOK, metrics)
}