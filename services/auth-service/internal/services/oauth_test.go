package services

import (
	"bytes"
	"context"
	"encoding/json"
	"io"
	"net/http"
	"testing"
	"time"

	"auth-service/internal/config"
)

type roundTripFunc func(*http.Request) (*http.Response, error)

func (fn roundTripFunc) RoundTrip(req *http.Request) (*http.Response, error) {
	return fn(req)
}

func testOAuthService(handler func(*testing.T, *http.Request) map[string]any) *OAuthService {
	return newOAuthService(config.OAuthConfig{
		GoogleClientID: "web-client-id,android-client-id",
	}, &http.Client{Transport: roundTripFunc(func(req *http.Request) (*http.Response, error) {
		fields := handler(nil, req)
		body, err := json.Marshal(fields)
		if err != nil {
			return nil, err
		}
		return &http.Response{
			StatusCode: http.StatusOK,
			Header:     make(http.Header),
			Body:       io.NopCloser(bytes.NewReader(body)),
		}, nil
	})}, "https://tokeninfo.test")
}

func testOAuthServiceWithT(t *testing.T, handler func(*testing.T, *http.Request) map[string]any) *OAuthService {
	t.Helper()
	return newOAuthService(config.OAuthConfig{
		GoogleClientID: "web-client-id,android-client-id",
	}, &http.Client{Transport: roundTripFunc(func(req *http.Request) (*http.Response, error) {
		fields := handler(t, req)
		body, err := json.Marshal(fields)
		if err != nil {
			return nil, err
		}
		return &http.Response{
			StatusCode: http.StatusOK,
			Header:     make(http.Header),
			Body:       io.NopCloser(bytes.NewReader(body)),
		}, nil
	})}, "https://tokeninfo.test")
}

func TestVerifyGoogleIDTokenValidatesTokenInfo(t *testing.T) {
	service := testOAuthServiceWithT(t, func(t *testing.T, r *http.Request) map[string]any {
		if got := r.URL.Query().Get("id_token"); got != "valid-token" {
			t.Fatalf("id_token query = %q", got)
		}
		return map[string]any{
			"iss":            "https://accounts.google.com",
			"aud":            "android-client-id",
			"sub":            "google-sub-123",
			"email":          "USER@example.com",
			"email_verified": "true",
			"name":           "Usuario Google",
			"picture":        "https://example.com/avatar.jpg",
			"exp":            time.Now().Add(time.Hour).Unix(),
		}
	})

	info, err := service.VerifyGoogleIDToken(context.Background(), "valid-token")
	if err != nil {
		t.Fatalf("VerifyGoogleIDToken returned error: %v", err)
	}

	if info.Subject != "google-sub-123" {
		t.Fatalf("Subject = %q", info.Subject)
	}
	if info.Email != "user@example.com" {
		t.Fatalf("Email = %q", info.Email)
	}
	if !info.EmailVerified {
		t.Fatal("expected verified email")
	}
}

func TestVerifyGoogleIDTokenRejectsWrongAudience(t *testing.T) {
	service := testOAuthService(func(t *testing.T, r *http.Request) map[string]any {
		return map[string]any{
			"iss":            "https://accounts.google.com",
			"aud":            "other-client-id",
			"sub":            "google-sub-123",
			"email":          "user@example.com",
			"email_verified": true,
			"exp":            time.Now().Add(time.Hour).Unix(),
		}
	})

	if _, err := service.VerifyGoogleIDToken(context.Background(), "token"); err != ErrInvalidOAuthToken {
		t.Fatalf("error = %v, want ErrInvalidOAuthToken", err)
	}
}

func TestVerifyGoogleIDTokenRejectsExpiredToken(t *testing.T) {
	service := testOAuthService(func(t *testing.T, r *http.Request) map[string]any {
		return map[string]any{
			"iss":            "accounts.google.com",
			"aud":            "web-client-id",
			"sub":            "google-sub-123",
			"email":          "user@example.com",
			"email_verified": true,
			"exp":            time.Now().Add(-time.Minute).Unix(),
		}
	})

	if _, err := service.VerifyGoogleIDToken(context.Background(), "token"); err != ErrInvalidOAuthToken {
		t.Fatalf("error = %v, want ErrInvalidOAuthToken", err)
	}
}

func TestVerifyGoogleIDTokenRejectsUnverifiedEmail(t *testing.T) {
	service := testOAuthService(func(t *testing.T, r *http.Request) map[string]any {
		return map[string]any{
			"iss":            "accounts.google.com",
			"aud":            "web-client-id",
			"sub":            "google-sub-123",
			"email":          "user@example.com",
			"email_verified": false,
			"exp":            time.Now().Add(time.Hour).Unix(),
		}
	})

	if _, err := service.VerifyGoogleIDToken(context.Background(), "token"); err != ErrUnverifiedEmail {
		t.Fatalf("error = %v, want ErrUnverifiedEmail", err)
	}
}
