package services

import (
	"context"
	"errors"
	"testing"

	"auth-service/internal/config"

	"google.golang.org/api/idtoken"
)

func TestVerifyGoogleIDToken_Success(t *testing.T) {
	svc := NewOAuthService(config.OAuthConfig{GoogleClientID: "valid-client-id"})
	svc.idTokenValidator = func(ctx context.Context, idToken, audience string) (*idtoken.Payload, error) {
		return &idtoken.Payload{
			Issuer:   "https://accounts.google.com",
			Audience: "valid-client-id",
			Subject:  "google-sub-123",
			Claims: map[string]interface{}{
				"email":          "user@example.com",
				"email_verified": true,
				"name":           "Test User",
				"picture":        "https://example.com/avatar.png",
			},
		}, nil
	}

	info, err := svc.VerifyGoogleIDToken(context.Background(), "any-token")
	if err != nil {
		t.Fatalf("expected no error, got %v", err)
	}
	if info.ID != "google-sub-123" {
		t.Errorf("expected ID google-sub-123, got %s", info.ID)
	}
	if info.Email != "user@example.com" {
		t.Errorf("expected email user@example.com, got %s", info.Email)
	}
	if info.Name != "Test User" {
		t.Errorf("expected name Test User, got %s", info.Name)
	}
	if info.Picture != "https://example.com/avatar.png" {
		t.Errorf("expected picture, got %s", info.Picture)
	}
	if !info.VerifiedEmail {
		t.Error("expected verified_email true")
	}
}

func TestVerifyGoogleIDToken_AudienceMismatch(t *testing.T) {
	svc := NewOAuthService(config.OAuthConfig{GoogleClientID: "expected-client-id"})
	svc.idTokenValidator = func(ctx context.Context, idToken, audience string) (*idtoken.Payload, error) {
		return &idtoken.Payload{
			Issuer:   "https://accounts.google.com",
			Audience: "other-client-id",
			Subject:  "google-sub-123",
			Claims: map[string]interface{}{
				"email":          "user@example.com",
				"email_verified": true,
			},
		}, nil
	}

	_, err := svc.VerifyGoogleIDToken(context.Background(), "any-token")
	if err == nil {
		t.Fatal("expected error for audience mismatch")
	}
}

func TestVerifyGoogleIDToken_EmailNotVerified(t *testing.T) {
	svc := NewOAuthService(config.OAuthConfig{GoogleClientID: "valid-client-id"})
	svc.idTokenValidator = func(ctx context.Context, idToken, audience string) (*idtoken.Payload, error) {
		return &idtoken.Payload{
			Issuer:   "https://accounts.google.com",
			Audience: "valid-client-id",
			Subject:  "google-sub-123",
			Claims: map[string]interface{}{
				"email":          "user@example.com",
				"email_verified": false,
			},
		}, nil
	}

	_, err := svc.VerifyGoogleIDToken(context.Background(), "any-token")
	if err == nil {
		t.Fatal("expected error for unverified email")
	}
}

func TestVerifyGoogleIDToken_InvalidIssuer(t *testing.T) {
	svc := NewOAuthService(config.OAuthConfig{GoogleClientID: "valid-client-id"})
	svc.idTokenValidator = func(ctx context.Context, idToken, audience string) (*idtoken.Payload, error) {
		return &idtoken.Payload{
			Issuer:   "https://malicious-issuer.com",
			Audience: "valid-client-id",
			Subject:  "google-sub-123",
			Claims: map[string]interface{}{
				"email":          "user@example.com",
				"email_verified": true,
			},
		}, nil
	}

	_, err := svc.VerifyGoogleIDToken(context.Background(), "any-token")
	if err == nil {
		t.Fatal("expected error for invalid issuer")
	}
}

func TestVerifyGoogleIDToken_EmptySubject(t *testing.T) {
	svc := NewOAuthService(config.OAuthConfig{GoogleClientID: "valid-client-id"})
	svc.idTokenValidator = func(ctx context.Context, idToken, audience string) (*idtoken.Payload, error) {
		return &idtoken.Payload{
			Issuer:   "https://accounts.google.com",
			Audience: "valid-client-id",
			Subject:  "",
			Claims: map[string]interface{}{
				"email":          "user@example.com",
				"email_verified": true,
			},
		}, nil
	}

	_, err := svc.VerifyGoogleIDToken(context.Background(), "any-token")
	if err == nil {
		t.Fatal("expected error for empty subject")
	}
}

func TestVerifyGoogleIDToken_ClientIDNotConfigured(t *testing.T) {
	svc := NewOAuthService(config.OAuthConfig{GoogleClientID: ""})
	svc.idTokenValidator = func(ctx context.Context, idToken, audience string) (*idtoken.Payload, error) {
		t.Fatal("validator should not be called when Google Client ID is not configured")
		return nil, nil
	}

	_, err := svc.VerifyGoogleIDToken(context.Background(), "any-token")
	if err == nil {
		t.Fatal("expected error when Google Client ID is not configured")
	}
}

func TestVerifyGoogleIDToken_ValidatorError(t *testing.T) {
	svc := NewOAuthService(config.OAuthConfig{GoogleClientID: "valid-client-id"})
	svc.idTokenValidator = func(ctx context.Context, idToken, audience string) (*idtoken.Payload, error) {
		return nil, errors.New("network error")
	}

	_, err := svc.VerifyGoogleIDToken(context.Background(), "any-token")
	if err == nil {
		t.Fatal("expected error when validator fails")
	}
}
