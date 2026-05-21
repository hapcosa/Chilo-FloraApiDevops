#!/usr/bin/env bash
# =============================================================================
# migrate.sh — runner de migraciones SQL para especies-api
# =============================================================================
# Aplica archivos *.sql de services/especies-api/migrations/ en orden
# lexicográfico (00NN_*.sql). Cada migración se registra en la tabla
# schema_migrations dentro de la BD destino; las ya aplicadas se saltan.
#
# Configuración por variables de entorno (mismo nombre que usa el binario C++):
#   DB_HOST, DB_PORT, DB_NAME, DB_USER, DB_PASSWORD
#
# Uso:
#   ./scripts/migrate.sh
#   DB_HOST=postgres ./scripts/migrate.sh
#
# Salida:
#   exit 0 si todo OK (incluso si no había nada que aplicar)
#   exit != 0 si una migración falla (rollback automático por BEGIN/COMMIT)
# =============================================================================

set -euo pipefail

DB_HOST="${DB_HOST:-localhost}"
DB_PORT="${DB_PORT:-5432}"
DB_NAME="${DB_NAME:-chiloe_flora}"
DB_USER="${DB_USER:-postgres}"
DB_PASSWORD="${DB_PASSWORD:-postgres}"

# Resolver dir de migraciones relativo a este script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MIGRATIONS_DIR="$(cd "${SCRIPT_DIR}/../migrations" && pwd)"

export PGPASSWORD="${DB_PASSWORD}"

psql_cmd() {
    psql -v ON_ERROR_STOP=1 -h "${DB_HOST}" -p "${DB_PORT}" \
         -U "${DB_USER}" -d "${DB_NAME}" "$@"
}

log() {
    echo "[migrate] $*"
}

log "destino: ${DB_USER}@${DB_HOST}:${DB_PORT}/${DB_NAME}"
log "directorio: ${MIGRATIONS_DIR}"

# Tabla de tracking: idempotente
psql_cmd -c "
CREATE TABLE IF NOT EXISTS schema_migrations (
    version    VARCHAR(255) PRIMARY KEY,
    applied_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
" >/dev/null

shopt -s nullglob
applied_any=0
failed=0

for migration in "${MIGRATIONS_DIR}"/[0-9]*.sql; do
    version="$(basename "${migration}" .sql)"

    already=$(psql_cmd -tAc \
        "SELECT 1 FROM schema_migrations WHERE version = '${version}'")

    if [[ "${already}" == "1" ]]; then
        log "skip   ${version} (ya aplicada)"
        continue
    fi

    log "apply  ${version}"
    if psql_cmd -f "${migration}"; then
        psql_cmd -c "INSERT INTO schema_migrations(version) VALUES ('${version}')" \
            >/dev/null
        applied_any=1
    else
        log "ERROR  ${version} falló"
        failed=1
        break
    fi
done

if [[ "${failed}" == "1" ]]; then
    log "abortado por error"
    exit 1
fi

if [[ "${applied_any}" == "0" ]]; then
    log "nada que aplicar"
fi

log "OK"
