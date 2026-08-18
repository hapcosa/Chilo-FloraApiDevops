// routes.go - Actualizar para agregar nuevas rutas
package api

import (
	"auth-service/internal/api/handlers"
	"auth-service/internal/middleware"
	"auth-service/internal/services"

	"github.com/gin-gonic/gin"
)

// SetupRoutes configura todas las rutas de la API
func SetupRoutes(router *gin.Engine, authService *services.AuthService, oauthService *services.OAuthService) {
	// Crear handlers
	authHandler := handlers.NewAuthHandler(authService, oauthService)
	healthHandler := handlers.NewHealthHandler()

	// ===== ENDPOINTS DE SALUD Y MÉTRICAS =====
	// Estos endpoints deben estar en la raíz para que nginx y otros servicios puedan accederlos fácilmente
	router.GET("/health", healthHandler.HealthCheck)
	router.GET("/ready", healthHandler.ReadinessCheck)
	router.GET("/alive", healthHandler.LivenessCheck)
	router.GET("/metrics", healthHandler.Metrics)

	// También agregar bajo /api/v1 por consistencia
	api := router.Group("/api/v1")
	{
		api.GET("/health", healthHandler.HealthCheck)
		api.GET("/metrics", healthHandler.Metrics)
	}

	// ===== RUTAS DE AUTENTICACIÓN =====
	// Grupo de rutas de autenticación
	auth := api.Group("/auth")
	{
		// Rutas públicas (sin autenticación)
		auth.POST("/register", authHandler.Register)
		auth.POST("/login", authHandler.Login)
		auth.POST("/refresh", authHandler.RefreshToken)
		auth.POST("/google", authHandler.GoogleIDTokenLogin)

		// OAuth endpoints
		auth.GET("/google", authHandler.GoogleAuth)
		auth.GET("/github", authHandler.GitHubAuth)
		auth.GET("/callback/google", authHandler.GoogleCallback)
		auth.GET("/callback/github", authHandler.GitHubCallback)

		// Endpoint CRÍTICO para verificación de tokens (usado por nginx)
		auth.GET("/verify", authHandler.VerifyToken)

		// Endpoint para obtener info del usuario actual (útil para frontend)
		auth.GET("/whoami", authHandler.WhoAmI)

		// Rutas protegidas (requieren autenticación)
		protected := auth.Group("/")
		protected.Use(middleware.AuthRequired(authService))
		{
			// Perfil de usuario
			protected.GET("/me", authHandler.GetProfile)
			protected.PUT("/me", authHandler.UpdateProfile)

			// Vista pública de otra persona. Va protegida: la comunidad es
			// de gente con cuenta, no un directorio abierto.
			protected.GET("/usuarios/:id/publico", authHandler.GetPerfilPublico)
			protected.POST("/change-password", authHandler.ChangePassword)

			// Logout
			protected.POST("/logout", authHandler.Logout)

			// Rutas de administración
			admin := protected.Group("/admin")
			admin.Use(middleware.AdminRequired())
			{
				admin.GET("/users", authHandler.ListUsers)
				admin.GET("/users/:id", authHandler.GetUser)
				admin.PUT("/users/:id", authHandler.UpdateUser)
				admin.DELETE("/users/:id", authHandler.DeleteUser)
				admin.POST("/users/:id/activate", authHandler.ActivateUser)
				admin.POST("/users/:id/deactivate", authHandler.DeactivateUser)
			}
		}
	}
}
