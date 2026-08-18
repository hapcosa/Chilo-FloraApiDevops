package models

import (
	"time"

	"gorm.io/gorm"
)

type User struct {
	ID            uint       `json:"id" gorm:"primarykey"`
	Email         string     `json:"email" gorm:"uniqueIndex;not null"`
	Password      string     `json:"-" gorm:"not null"` // No incluir en JSON
	Name          string     `json:"name" gorm:"not null"`
	Avatar        string     `json:"avatar"`
	Role          UserRole   `json:"role" gorm:"default:user"`
	Status        UserStatus `json:"status" gorm:"default:active"`
	Provider      string     `json:"provider" gorm:"default:local"` // local, google, github
	ProviderID    string     `json:"provider_id"`
	GoogleSub     *string    `json:"-" gorm:"uniqueIndex"`
	EmailVerified bool       `json:"email_verified" gorm:"default:false"`
	// Presentación de la persona. `Profesion` da respaldo a quien modera, así
	// que solo se muestra a terceros cuando el rol lo justifica.
	Bio       string `json:"bio" gorm:"type:varchar(500)"`
	Profesion string `json:"profesion" gorm:"type:varchar(120)"`
	// Privado por defecto: la vista pública de perfil es una decisión de la
	// persona, no un estado inicial.
	PerfilPublico bool           `json:"perfil_publico" gorm:"not null;default:false"`
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

// RegisterRequest para registro
type RegisterRequest struct {
	Email    string `json:"email" binding:"required,email"`
	Password string `json:"password" binding:"required,min=8"`
	Name     string `json:"name" binding:"required,min=2"`
}

// UpdateUserRequest para actualizar perfil.
//
// Los campos nuevos son punteros porque un string vacío es un valor válido:
// borrar la bio tiene que ser distinguible de no haberla enviado.
type UpdateUserRequest struct {
	Name          string  `json:"name" binding:"omitempty,min=2"`
	Avatar        string  `json:"avatar" binding:"omitempty,url"`
	Bio           *string `json:"bio" binding:"omitempty,max=500"`
	Profesion     *string `json:"profesion" binding:"omitempty,max=120"`
	PerfilPublico *bool   `json:"perfil_publico"`
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
	Bio           string     `json:"bio"`
	Profesion     string     `json:"profesion"`
	PerfilPublico bool       `json:"perfil_publico"`
	CreatedAt     time.Time  `json:"created_at"`
}

// UserPerfilPublico es lo que ve un tercero. Nunca lleva email ni estado de
// la cuenta: identifica a una persona ante la comunidad, no ante el sistema.
type UserPerfilPublico struct {
	ID        uint      `json:"id"`
	Name      string    `json:"name"`
	Avatar    string    `json:"avatar"`
	Role      UserRole  `json:"role"`
	Bio       string    `json:"bio"`
	Profesion string    `json:"profesion,omitempty"`
	CreatedAt time.Time `json:"created_at"`
}

// muestraProfesion: la profesión existe para respaldar a quien modera. En una
// cuenta corriente sería un dato personal publicado sin justificación.
func (u *User) muestraProfesion() bool {
	return u.Role == UserRoleModerator || u.Role == UserRoleAdmin || u.Role == UserRoleResearcher
}

// ToPerfilPublico convierte User en la vista que ven terceros.
func (u *User) ToPerfilPublico() UserPerfilPublico {
	perfil := UserPerfilPublico{
		ID:        u.ID,
		Name:      u.Name,
		Avatar:    u.Avatar,
		Role:      u.Role,
		Bio:       u.Bio,
		CreatedAt: u.CreatedAt,
	}
	if u.muestraProfesion() {
		perfil.Profesion = u.Profesion
	}
	return perfil
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
		Bio:           u.Bio,
		Profesion:     u.Profesion,
		PerfilPublico: u.PerfilPublico,
		CreatedAt:     u.CreatedAt,
	}
}
