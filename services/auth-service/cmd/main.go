package main

import (
	"context"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"auth-service/internal/api"
	"auth-service/internal/config"
	"auth-service/internal/database"
	"auth-service/internal/middleware"
	"auth-service/internal/services"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
	"github.com/joho/godotenv"
)

func main() {
	// Cargar variables de entorno
	if err := godotenv.Load(); err != nil {
		log.Println("No .env file found, using environment variables")
	}

	// Cargar configuración
	cfg := config.Load()

	// Conectar a la base de datos
	db, err := database.Connect(cfg.Database)
	if err != nil {
		log.Fatalf("Failed to connect to database: %v", err)
	}

	// Ejecutar migraciones
	if err := database.Migrate(db, cfg.Environment); err != nil {
		log.Fatalf("Failed to run migrations: %v", err)
	}

	// Conectar a Redis
	redisClient, err := database.ConnectRedis(cfg.Redis)
	if err != nil {
		log.Printf("Failed to connect to Redis: %v", err)
		// Redis es opcional, continuar sin él
	}

	// Inicializar servicios
	authService := services.NewAuthService(db, redisClient, cfg.JWT)
	oauthService := services.NewOAuthService(cfg.OAuth)

	// Configurar Gin
	if cfg.Environment == "production" {
		gin.SetMode(gin.ReleaseMode)
	}

	router := gin.New()

	// Middleware global
	router.Use(gin.Logger())
	router.Use(gin.Recovery())
	router.Use(middleware.RequestID())

	// CORS
	corsConfig := cors.Config{
		AllowOrigins:     []string{"*"}, // En producción, especificar orígenes
		AllowMethods:     []string{"GET", "POST", "PUT", "DELETE", "PATCH", "OPTIONS"},
		AllowHeaders:     []string{"Authorization", "Content-Type", "Accept", "Origin", "User-Agent", "X-Requested-With"},
		ExposeHeaders:    []string{"X-User-ID", "X-User", "X-Total-Count"},
		AllowCredentials: true,
		MaxAge:           12 * time.Hour,
	}
	router.Use(cors.New(corsConfig))

	// API routes
	api.SetupRoutes(router, authService, oauthService)

	// Configurar servidor HTTP
	server := &http.Server{
		Addr:    fmt.Sprintf(":%s", cfg.API.Port),
		Handler: router,
	}

	// Iniciar servidor en goroutine
	go func() {
		log.Printf("🚀 Auth Service starting on port %s", cfg.API.Port)
		log.Printf("🌿 Environment: %s", cfg.Environment)
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("Failed to start server: %v", err)
		}
	}()

	// Graceful shutdown
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit

	log.Println("🛑 Shutting down server...")

	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	if err := server.Shutdown(ctx); err != nil {
		log.Fatalf("Server forced to shutdown: %v", err)
	}

	// Cerrar conexiones
	if redisClient != nil {
		redisClient.Close()
	}

	if sqlDB, err := db.DB(); err == nil {
		sqlDB.Close()
	}

	log.Println("✅ Server exited")
}
