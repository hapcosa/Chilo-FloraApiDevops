package middleware

import (
	"strconv"
	"strings"

	"auth-service/internal/models"
	"auth-service/internal/services"

	"github.com/gin-gonic/gin"
	"github.com/google/uuid"
)

// RequestID añade un ID único a cada request
func RequestID() gin.HandlerFunc {
	return func(c *gin.Context) {
		requestID := c.GetHeader("X-Request-ID")
		if requestID == "" {
			requestID = uuid.New().String()
		}
		c.Header("X-Request-ID", requestID)
		c.Set("request_id", requestID)
		c.Next()
	}
}

// AuthRequired middleware que requiere autenticación
func AuthRequired(authService *services.AuthService) gin.HandlerFunc {
	return func(c *gin.Context) {
		// Obtener token del header Authorization
		authHeader := c.GetHeader("Authorization")
		if authHeader == "" {
			c.JSON(401, gin.H{
				"error":   "Unauthorized",
				"message": "Missing authorization header",
				"code":    401,
			})
			c.Abort()
			return
		}

		// Verificar formato Bearer
		tokenParts := strings.SplitN(authHeader, " ", 2)
		if len(tokenParts) != 2 || tokenParts[0] != "Bearer" {
			c.JSON(401, gin.H{
				"error":   "Unauthorized",
				"message": "Invalid authorization header format",
				"code":    401,
			})
			c.Abort()
			return
		}

		// Verificar token
		user, err := authService.VerifyToken(tokenParts[1])
		if err != nil {
			c.JSON(401, gin.H{
				"error":   "Unauthorized",
				"message": "Invalid or expired token",
				"code":    401,
			})
			c.Abort()
			return
		}

		// Agregar usuario al contexto
		c.Set("user", user)
		c.Set("user_id", user.ID)

		// Headers para nginx
		c.Header("X-User-ID", strconv.Itoa(int(user.ID)))
		c.Header("X-User", user.Email)

		c.Next()
	}
}

// RoleRequired middleware que requiere un rol específico
func RoleRequired(roles ...models.UserRole) gin.HandlerFunc {
	return func(c *gin.Context) {
		user, exists := c.Get("user")
		if !exists {
			c.JSON(401, gin.H{
				"error":   "Unauthorized",
				"message": "User not found in context",
				"code":    401,
			})
			c.Abort()
			return
		}

		userModel, ok := user.(*models.User)
		if !ok {
			c.JSON(500, gin.H{
				"error":   "Internal Server Error",
				"message": "Invalid user type in context",
				"code":    500,
			})
			c.Abort()
			return
		}

		// Verificar si el usuario tiene alguno de los roles permitidos
		hasRole := false
		for _, role := range roles {
			if userModel.Role == role {
				hasRole = true
				break
			}
		}

		if !hasRole {
			c.JSON(403, gin.H{
				"error":   "Forbidden",
				"message": "Insufficient permissions",
				"code":    403,
			})
			c.Abort()
			return
		}

		c.Next()
	}
}

// AdminRequired middleware que requiere rol de administrador
func AdminRequired() gin.HandlerFunc {
	return RoleRequired(models.UserRoleAdmin)
}

// ModeratorOrAdminRequired middleware que requiere rol de moderador o admin
func ModeratorOrAdminRequired() gin.HandlerFunc {
	return RoleRequired(models.UserRoleAdmin, models.UserRoleModerator)
}

// GetCurrentUser helper para obtener el usuario actual del contexto
func GetCurrentUser(c *gin.Context) (*models.User, bool) {
	user, exists := c.Get("user")
	if !exists {
		return nil, false
	}

	userModel, ok := user.(*models.User)
	return userModel, ok
}

// GetCurrentUserID helper para obtener el ID del usuario actual
func GetCurrentUserID(c *gin.Context) (uint, bool) {
	userID, exists := c.Get("user_id")
	if !exists {
		return 0, false
	}

	id, ok := userID.(uint)
	return id, ok
}
