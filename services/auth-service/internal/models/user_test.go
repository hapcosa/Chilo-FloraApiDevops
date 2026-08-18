package models

import "testing"

func TestToPerfilPublicoOcultaDatosDeCuenta(t *testing.T) {
	user := User{
		ID:        7,
		Email:     "persona@ejemplo.cl",
		Name:      "Persona",
		Bio:       "Observadora de aves en Chiloé",
		Profesion: "Bióloga",
		Role:      UserRoleUser,
		Status:    UserStatusActive,
	}

	perfil := user.ToPerfilPublico()

	if perfil.ID != 7 || perfil.Name != "Persona" {
		t.Fatalf("perfil no conserva identidad: %+v", perfil)
	}
	if perfil.Bio != "Observadora de aves en Chiloé" {
		t.Errorf("bio = %q, se esperaba la del usuario", perfil.Bio)
	}
	// La profesión de una cuenta corriente es un dato personal sin
	// justificación: solo respalda a quien modera.
	if perfil.Profesion != "" {
		t.Errorf("profesion = %q, se esperaba vacía para rol user", perfil.Profesion)
	}
}

func TestToPerfilPublicoMuestraProfesionCuandoElRolLaJustifica(t *testing.T) {
	for _, rol := range []UserRole{UserRoleModerator, UserRoleAdmin, UserRoleResearcher} {
		user := User{Role: rol, Profesion: "Micóloga"}
		if got := user.ToPerfilPublico().Profesion; got != "Micóloga" {
			t.Errorf("rol %s: profesion = %q, se esperaba %q", rol, got, "Micóloga")
		}
	}
}
