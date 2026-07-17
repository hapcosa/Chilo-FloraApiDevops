package database

import (
	"context"
	"fmt"
	"log"

	"auth-service/internal/config"
	"auth-service/internal/models"

	"github.com/redis/go-redis/v9"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
	"gorm.io/gorm/logger"
)

// Connect establece conexión con PostgreSQL
func Connect(cfg config.DatabaseConfig) (*gorm.DB, error) {
	dsn := fmt.Sprintf(
		"host=%s user=%s password=%s dbname=%s port=%s sslmode=%s TimeZone=America/Santiago",
		cfg.Host, cfg.User, cfg.Password, cfg.Name, cfg.Port, cfg.SSLMode,
	)

	// Configurar logger para desarrollo
	logLevel := logger.Silent
	if cfg.SSLMode == "disable" { // Asumimos desarrollo si SSL está deshabilitado
		logLevel = logger.Info
	}

	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{
		Logger: logger.Default.LogMode(logLevel),
	})

	if err != nil {
		return nil, fmt.Errorf("failed to connect to database: %w", err)
	}

	// Configurar pool de conexiones
	sqlDB, err := db.DB()
	if err != nil {
		return nil, fmt.Errorf("failed to get database instance: %w", err)
	}

	// Configuración del pool
	sqlDB.SetMaxIdleConns(10)
	sqlDB.SetMaxOpenConns(100)

	log.Println("✅ Connected to PostgreSQL database")
	return db, nil
}

// ConnectRedis establece conexión con Redis
func ConnectRedis(cfg config.RedisConfig) (*redis.Client, error) {
	rdb := redis.NewClient(&redis.Options{
		Addr:     fmt.Sprintf("%s:%s", cfg.Host, cfg.Port),
		Password: cfg.Password,
		DB:       cfg.DB,
	})

	// Test connection
	ctx := context.Background()
	if err := rdb.Ping(ctx).Err(); err != nil {
		return nil, fmt.Errorf("failed to connect to Redis: %w", err)
	}

	log.Println("✅ Connected to Redis")
	return rdb, nil
}

// Migrate ejecuta las migraciones de la base de datos
func Migrate(db *gorm.DB) error {
	log.Println("🔄 Running database migrations...")

	// Crear extensiones si no existen
	if err := db.Exec("CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\"").Error; err != nil {
		return fmt.Errorf("failed to create uuid extension: %w", err)
	}

	// Ejecutar migraciones automáticas
	if err := db.AutoMigrate(
		&models.User{},
		&models.RefreshToken{},
	); err != nil {
		return fmt.Errorf("failed to run auto migrations: %w", err)
	}

	// Crear índices adicionales si son necesarios
	if err := createIndexes(db); err != nil {
		return fmt.Errorf("failed to create indexes: %w", err)
	}

	// Crear usuario admin por defecto si no existe
	if err := createDefaultAdmin(db); err != nil {
		return fmt.Errorf("failed to create default admin: %w", err)
	}

	log.Println("✅ Database migrations completed")
	return nil
}

// createIndexes crea índices adicionales para mejorar rendimiento
func createIndexes(db *gorm.DB) error {
	// Índices adicionales para usuarios
	indexes := []string{
		"CREATE INDEX IF NOT EXISTS idx_users_provider_id ON users(provider, provider_id)",
		"CREATE INDEX IF NOT EXISTS idx_users_status_role ON users(status, role)",
		"CREATE UNIQUE INDEX IF NOT EXISTS idx_users_google_sub ON users(google_sub) WHERE google_sub IS NOT NULL",
		"CREATE INDEX IF NOT EXISTS idx_refresh_tokens_expires_at ON refresh_tokens(expires_at)",
	}

	for _, idx := range indexes {
		if err := db.Exec(idx).Error; err != nil {
			return fmt.Errorf("failed to create index: %w", err)
		}
	}

	return nil
}

// createDefaultAdmin crea un usuario admin por defecto
func createDefaultAdmin(db *gorm.DB) error {
	// Verificar si ya existe un admin
	var count int64
	if err := db.Model(&models.User{}).Where("role = ?", models.UserRoleAdmin).Count(&count).Error; err != nil {
		return err
	}

	// Si ya existe un admin, no crear otro
	if count > 0 {
		return nil
	}

	// Crear admin por defecto (solo en desarrollo)
	adminUser := models.User{
		Email:         "admin@chiloe.dev",
		Password:      "$2a$10$92IXUNpkjO0rOQ5byMi.Ye4oKoEa3Ro9llC/.og/at2.uheWG/igi", // "admin123"
		Name:          "Administrador Flora Chiloé",
		Role:          models.UserRoleAdmin,
		Status:        models.UserStatusActive,
		Provider:      "local",
		EmailVerified: true,
	}

	if err := db.Create(&adminUser).Error; err != nil {
		return fmt.Errorf("failed to create default admin: %w", err)
	}

	log.Println("👤 Default admin user created: admin@chiloe.dev")
	return nil
}
