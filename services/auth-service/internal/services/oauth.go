package services

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"

	"auth-service/internal/config"
)

type OAuthService struct {
	config config.OAuthConfig
}

type GoogleUserInfo struct {
	ID            string `json:"id"`
	Email         string `json:"email"`
	VerifiedEmail bool   `json:"verified_email"`
	Name          string `json:"name"`
	GivenName     string `json:"given_name"`
	FamilyName    string `json:"family_name"`
	Picture       string `json:"picture"`
}

type GitHubUserInfo struct {
	ID        int    `json:"id"`
	Login     string `json:"login"`
	Name      string `json:"name"`
	Email     string `json:"email"`
	AvatarURL string `json:"avatar_url"`
}

func NewOAuthService(config config.OAuthConfig) *OAuthService {
	return &OAuthService{
		config: config,
	}
}

// GetGoogleAuthURL genera la URL de autenticación de Google
func (s *OAuthService) GetGoogleAuthURL(state string) string {
	if s.config.GoogleClientID == "" {
		return ""
	}

	baseURL := "https://accounts.google.com/o/oauth2/auth"
	params := url.Values{
		"client_id":     {s.config.GoogleClientID},
		"redirect_uri":  {s.config.RedirectURL + "/google"},
		"response_type": {"code"},
		"scope":         {"openid email profile"},
		"state":         {state},
		"access_type":   {"offline"},
		"prompt":        {"consent"},
	}

	return baseURL + "?" + params.Encode()
}

// GetGitHubAuthURL genera la URL de autenticación de GitHub
func (s *OAuthService) GetGitHubAuthURL(state string) string {
	if s.config.GitHubClientID == "" {
		return ""
	}

	baseURL := "https://github.com/login/oauth/authorize"
	params := url.Values{
		"client_id":    {s.config.GitHubClientID},
		"redirect_uri": {s.config.RedirectURL + "/github"},
		"scope":        {"user:email"},
		"state":        {state},
	}

	return baseURL + "?" + params.Encode()
}

// ExchangeGoogleCode intercambia el código de autorización por información del usuario
func (s *OAuthService) ExchangeGoogleCode(code string) (*GoogleUserInfo, error) {
	// Intercambiar código por token
	tokenURL := "https://oauth2.googleapis.com/token"
	tokenData := url.Values{
		"client_id":     {s.config.GoogleClientID},
		"client_secret": {s.config.GoogleClientSecret},
		"code":          {code},
		"grant_type":    {"authorization_code"},
		"redirect_uri":  {s.config.RedirectURL + "/google"},
	}

	tokenResp, err := http.PostForm(tokenURL, tokenData)
	if err != nil {
		return nil, fmt.Errorf("failed to exchange code for token: %w", err)
	}
	defer tokenResp.Body.Close()

	var tokenResult struct {
		AccessToken string `json:"access_token"`
		TokenType   string `json:"token_type"`
	}

	if err := json.NewDecoder(tokenResp.Body).Decode(&tokenResult); err != nil {
		return nil, fmt.Errorf("failed to decode token response: %w", err)
	}

	// Obtener información del usuario
	userInfoURL := "https://www.googleapis.com/oauth2/v2/userinfo"
	req, err := http.NewRequest("GET", userInfoURL, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create user info request: %w", err)
	}

	req.Header.Set("Authorization", fmt.Sprintf("Bearer %s", tokenResult.AccessToken))

	client := &http.Client{}
	userResp, err := client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to get user info: %w", err)
	}
	defer userResp.Body.Close()

	if userResp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(userResp.Body)
		return nil, fmt.Errorf("failed to get user info, status: %d, body: %s", userResp.StatusCode, string(body))
	}

	var userInfo GoogleUserInfo
	if err := json.NewDecoder(userResp.Body).Decode(&userInfo); err != nil {
		return nil, fmt.Errorf("failed to decode user info: %w", err)
	}

	return &userInfo, nil
}

// ExchangeGitHubCode intercambia el código de autorización por información del usuario
func (s *OAuthService) ExchangeGitHubCode(code string) (*GitHubUserInfo, error) {
	// Intercambiar código por token
	tokenURL := "https://github.com/login/oauth/access_token"
	tokenData := url.Values{
		"client_id":     {s.config.GitHubClientID},
		"client_secret": {s.config.GitHubClientSecret},
		"code":          {code},
	}

	req, err := http.NewRequest("POST", tokenURL, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create token request: %w", err)
	}

	req.URL.RawQuery = tokenData.Encode()
	req.Header.Set("Accept", "application/json")

	client := &http.Client{}
	tokenResp, err := client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("failed to exchange code for token: %w", err)
	}
	defer tokenResp.Body.Close()

	var tokenResult struct {
		AccessToken string `json:"access_token"`
		TokenType   string `json:"token_type"`
		Scope       string `json:"scope"`
	}

	if err := json.NewDecoder(tokenResp.Body).Decode(&tokenResult); err != nil {
		return nil, fmt.Errorf("failed to decode token response: %w", err)
	}

	if tokenResult.AccessToken == "" {
		return nil, fmt.Errorf("no access token received")
	}

	// Obtener información del usuario
	userInfoURL := "https://api.github.com/user"
	userReq, err := http.NewRequest("GET", userInfoURL, nil)
	if err != nil {
		return nil, fmt.Errorf("failed to create user info request: %w", err)
	}

	userReq.Header.Set("Authorization", fmt.Sprintf("token %s", tokenResult.AccessToken))
	userReq.Header.Set("Accept", "application/vnd.github.v3+json")

	userResp, err := client.Do(userReq)
	if err != nil {
		return nil, fmt.Errorf("failed to get user info: %w", err)
	}
	defer userResp.Body.Close()

	if userResp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(userResp.Body)
		return nil, fmt.Errorf("failed to get user info, status: %d, body: %s", userResp.StatusCode, string(body))
	}

	var userInfo GitHubUserInfo
	if err := json.NewDecoder(userResp.Body).Decode(&userInfo); err != nil {
		return nil, fmt.Errorf("failed to decode user info: %w", err)
	}

	// Si no hay email público, obtener emails privados
	if userInfo.Email == "" {
		if email, err := s.getGitHubPrimaryEmail(tokenResult.AccessToken); err == nil {
			userInfo.Email = email
		}
	}

	return &userInfo, nil
}

// getGitHubPrimaryEmail obtiene el email principal del usuario de GitHub
func (s *OAuthService) getGitHubPrimaryEmail(token string) (string, error) {
	emailURL := "https://api.github.com/user/emails"
	req, err := http.NewRequest("GET", emailURL, nil)
	if err != nil {
		return "", err
	}

	req.Header.Set("Authorization", fmt.Sprintf("token %s", token))
	req.Header.Set("Accept", "application/vnd.github.v3+json")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("failed to get emails, status: %d", resp.StatusCode)
	}

	var emails []struct {
		Email    string `json:"email"`
		Primary  bool   `json:"primary"`
		Verified bool   `json:"verified"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&emails); err != nil {
		return "", err
	}

	// Buscar email principal y verificado
	for _, email := range emails {
		if email.Primary && email.Verified {
			return email.Email, nil
		}
	}

	// Si no hay principal, usar el primer verificado
	for _, email := range emails {
		if email.Verified {
			return email.Email, nil
		}
	}

	return "", fmt.Errorf("no verified email found")
}
