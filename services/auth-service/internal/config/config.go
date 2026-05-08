package config

import (
	"os"
	"time"
)

type Config struct {
	Environment string
	API         APIConfig
	Database    DatabaseConfig
	Redis       RedisConfig
	JWT         JWTConfig
	OAuth       OAuthConfig
}

type APIConfig struct {
	Port string
	Host string
}

type DatabaseConfig struct {
	Host     string
	Port     string
	Name     string
	User     string
	Password string
	SSLMode  string
}

type RedisConfig struct {
	Host     string
	Port     string
	Password string
	DB       int
}

type JWTConfig struct {
	Secret        string
	Expiry        time.Duration
	RefreshExpiry time.Duration
}

type OAuthConfig struct {
	GoogleClientID     string
	GoogleClientSecret string
	GitHubClientID     string
	GitHubClientSecret string
	RedirectURL        string
}

func Load() *Config {
	return &Config{
		Environment: getEnv("ENVIRONMENT", "development"),
		API: APIConfig{
			Port: getEnv("API_PORT", "8081"),
			Host: getEnv("API_HOST", "0.0.0.0"),
		},
		Database: DatabaseConfig{
			Host:     getEnv("DB_HOST", "localhost"),
			Port:     getEnv("DB_PORT", "5432"),
			Name:     getEnv("DB_NAME", "chiloe_flora_dev"),
			User:     getEnv("DB_USER", "dev_user"),
			Password: getEnv("DB_PASSWORD", "dev_password"),
			SSLMode:  getEnv("DB_SSLMODE", "disable"),
		},
		Redis: RedisConfig{
			Host:     getEnv("REDIS_HOST", "localhost"),
			Port:     getEnv("REDIS_PORT", "6379"),
			Password: getEnv("REDIS_PASSWORD", ""),
			DB:       0,
		},
		JWT: JWTConfig{
			Secret:        getEnv("JWT_SECRET", "your-super-secret-jwt-key-change-in-production"),
			Expiry:        parseDuration(getEnv("JWT_EXPIRY", "24h")),
			RefreshExpiry: parseDuration(getEnv("JWT_REFRESH_EXPIRY", "168h")), // 7 days
		},
		OAuth: OAuthConfig{
			GoogleClientID:     getEnv("GOOGLE_CLIENT_ID", ""),
			GoogleClientSecret: getEnv("GOOGLE_CLIENT_SECRET", ""),
			GitHubClientID:     getEnv("GITHUB_CLIENT_ID", ""),
			GitHubClientSecret: getEnv("GITHUB_CLIENT_SECRET", ""),
			RedirectURL:        getEnv("OAUTH_REDIRECT_URL", "http://localhost:8080/api/v1/auth/callback"),
		},
	}
}

func getEnv(key, defaultValue string) string {
	if value := os.Getenv(key); value != "" {
		return value
	}
	return defaultValue
}

func parseDuration(s string) time.Duration {
	d, err := time.ParseDuration(s)
	if err != nil {
		return 24 * time.Hour // default to 24 hours
	}
	return d
}
