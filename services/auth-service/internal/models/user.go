package models

import (
	"time"

	"gorm.io/gorm"
)

type User struct {
	ID            uint           `json:"id" gorm:"primarykey"`
	Email         string         `json:"email" gorm:"uniqueIndex;not null"`
	Password      string         `json:"-" gorm:"not null"` // No incluir en JSON
	Name          string         `json:"name" gorm:"not null"`
	Avatar        string         `json:"avatar"`
	Role          UserRole       `json:"role" gorm:"default:user"`
	Status        UserStatus     `json:"status" gorm:"default:active"`
	Provider      string         `json:"provider" gorm:"default:local"` // local, google, github
	ProviderID    string         `json:"provider_id"`
	EmailVerified bool           `json:"email_verified" gorm:"default:false"`
	CreatedAt     time.Time      `json:"created_at"`
	UpdatedAt     time.Time      `json:"updated_at"`
	DeletedAt     gorm.DeletedAt `json:"-" gorm:"index"`

	// Relaciones con otros servicios (opcional, para futuro)
	// Contributions []Contribution `json:"contributions,omitempty" gorm:"foreignKey:UserID"`
}

type UserRole string

const (
	UserRoleAdmin      UserRole = "admin"
	UserRoleModerator  UserRole = "moderator"
	UserRoleResearcher UserRole = "researcher"
	UserRoleUser       UserRole = "user"
)

type UserStatus string

const (
	UserStatusActive    UserStatus = "active"
	UserStatusInactive  UserStatus = "inactive"
	UserStatusSuspended UserStatus = "suspended"
)

// RefreshToken para manejar tokens de actualización
type RefreshToken struct {
	ID        uint      `json:"id" gorm:"primarykey"`
	UserID    uint      `json:"user_id" gorm:"not null;index"`
	Token     string    `json:"-" gorm:"not null;unique"` // Hash del token
	ExpiresAt time.Time `json:"expires_at" gorm:"not null"`
	CreatedAt time.Time `json:"created_at"`
	User      User      `json:"user" gorm:"foreignKey:UserID"`
}

// LoginRequest para autenticación
type LoginRequest struct {
	Email    string `json:"email" binding:"required,email"`
	Password string `json:"password" binding:"required,min=6"`
}

// GoogleLoginRequest para login con idToken de Google Sign-In
type GoogleLoginRequest struct {
	IDToken string `json:"id_token" binding:"required"`
}

// RegisterRequest para registro
type RegisterRequest struct {
	Email    string `json:"email" binding:"required,email"`
	Password string `json:"password" binding:"required,min=8"`
	Name     string `json:"name" binding:"required,min=2"`
}

// UpdateUserRequest para actualizar perfil
type UpdateUserRequest struct {
	Name   string `json:"name" binding:"omitempty,min=2"`
	Avatar string `json:"avatar" binding:"omitempty,url"`
}

// ChangePasswordRequest para cambiar contraseña
type ChangePasswordRequest struct {
	CurrentPassword string `json:"current_password" binding:"required"`
	NewPassword     string `json:"new_password" binding:"required,min=8"`
}

// AuthResponse respuesta de autenticación
type AuthResponse struct {
	User         UserPublic `json:"user"`
	AccessToken  string     `json:"access_token"`
	RefreshToken string     `json:"refresh_token"`
	ExpiresIn    int64      `json:"expires_in"` // segundos
}

// UserPublic información pública del usuario
type UserPublic struct {
	ID            uint       `json:"id"`
	Email         string     `json:"email"`
	Name          string     `json:"name"`
	Avatar        string     `json:"avatar"`
	Role          UserRole   `json:"role"`
	Status        UserStatus `json:"status"`
	Provider      string     `json:"provider"`
	EmailVerified bool       `json:"email_verified"`
	CreatedAt     time.Time  `json:"created_at"`
}

// ToPublic convierte User a UserPublic
func (u *User) ToPublic() UserPublic {
	return UserPublic{
		ID:            u.ID,
		Email:         u.Email,
		Name:          u.Name,
		Avatar:        u.Avatar,
		Role:          u.Role,
		Status:        u.Status,
		Provider:      u.Provider,
		EmailVerified: u.EmailVerified,
		CreatedAt:     u.CreatedAt,
	}
}
