# infrastructure/terraform/secrets.tf

# Crear secrets en AWS Secrets Manager
resource "aws_secretsmanager_secret" "db_credentials" {
  name                    = "${var.project_name}-db-credentials"
  description             = "Database credentials for Chiloe Flora"
  recovery_window_in_days = 7

  tags = {
    Environment = var.environment
    Project     = var.project_name
  }
}

resource "aws_secretsmanager_secret_version" "db_credentials" {
  secret_id = aws_secretsmanager_secret.db_credentials.id
  secret_string = jsonencode({
    username = var.db_username
    password = var.db_password
    host     = aws_db_instance.postgres.endpoint
    port     = aws_db_instance.postgres.port
    dbname   = aws_db_instance.postgres.db_name
  })
}

resource "aws_secretsmanager_secret" "jwt_secret" {
  name                    = "${var.project_name}-jwt-secret"
  description             = "JWT secret for authentication"
  recovery_window_in_days = 7

  tags = {
    Environment = var.environment
    Project     = var.project_name
  }
}

resource "aws_secretsmanager_secret_version" "jwt_secret" {
  secret_id = aws_secretsmanager_secret.jwt_secret.id
  secret_string = jsonencode({
    jwt_secret = var.jwt_secret
  })
}

# IAM role para que los pods accedan a secrets
resource "aws_iam_role" "secrets_role" {
  name = "${var.project_name}-secrets-role"

  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Action = "sts:AssumeRoleWithWebIdentity"
        Effect = "Allow"
        Principal = {
          Federated = aws_iam_openid_connect_provider.eks.arn
        }
        Condition = {
          StringEquals = {
            "${replace(aws_iam_openid_connect_provider.eks.url, "https://", "")}:sub" = "system:serviceaccount:chiloe-flora:secrets-service-account"
            "${replace(aws_iam_openid_connect_provider.eks.url, "https://", "")}:aud" = "sts.amazonaws.com"
          }
        }
      }
    ]
  })
}

resource "aws_iam_policy" "secrets_policy" {
  name = "${var.project_name}-secrets-policy"

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Effect = "Allow"
        Action = [
          "secretsmanager:GetSecretValue",
          "secretsmanager:DescribeSecret"
        ]
        Resource = [
          aws_secretsmanager_secret.db_credentials.arn,
          aws_secretsmanager_secret.jwt_secret.arn
        ]
      }
    ]
  })
}

resource "aws_iam_role_policy_attachment" "secrets_role_policy" {
  role       = aws_iam_role.secrets_role.name
  policy_arn = aws_iam_policy.secrets_policy.arn
}

# Variables para secrets (se pasan desde GitHub Actions)
variable "db_password" {
  description = "Database password"
  type        = string
  sensitive   = true
}

variable "jwt_secret" {
  description = "JWT secret key"
  type        = string
  sensitive   = true
}

# Outputs para usar en Kubernetes
output "db_secret_arn" {
  value = aws_secretsmanager_secret.db_credentials.arn
}

output "jwt_secret_arn" {
  value = aws_secretsmanager_secret.jwt_secret.arn
}