services/auth-service/
├── cmd/main.go
├── internal/
│   ├── api/
│   │   ├── routes.go
│   │   └── handlers/auth.go
│   ├── config/config.go
│   ├── database/database.go
│   ├── middleware/middleware.go
│   ├── models/user.go
│   └── services/
│       ├── auth.go
│       └── oauth.go
├── go.mod
├── Dockerfile.dev
├── .air.toml
└── .env.example