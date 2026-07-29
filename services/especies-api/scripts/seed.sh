#!/usr/bin/env bash
# =============================================================================
# seed.sh — carga de contenido inicial para especies-api
# =============================================================================
# Aplica los archivos *.sql de services/especies-api/seeds/ en orden
# lexicográfico. A diferencia de las migraciones, los seeds no se registran en
# ninguna tabla de tracking: están escritos para ser idempotentes
# (ON CONFLICT DO NOTHING), de modo que reaplicarlos no duplica ni pisa datos
# editados a mano.
#
# Requiere que las migraciones ya estén aplicadas (./scripts/migrate.sh).
#
# Configuración por variables de entorno (mismo nombre que usa el binario C++):
#   DB_HOST, DB_PORT, DB_NAME, DB_USER, DB_PASSWORD
#
# Uso:
#   ./scripts/seed.sh
#   DB_HOST=postgres ./scripts/seed.sh
#
# Salida:
#   exit 0 si todo OK
#   exit != 0 si un seed falla (rollback automático por BEGIN/COMMIT)
# =============================================================================

set -euo pipefail

DB_HOST="${DB_HOST:-localhost}"
DB_PORT="${DB_PORT:-5432}"
DB_NAME="${DB_NAME:-chiloe_flora}"
DB_USER="${DB_USER:-postgres}"
DB_PASSWORD="${DB_PASSWORD:-postgres}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SEEDS_DIR="$(cd "${SCRIPT_DIR}/../seeds" && pwd)"

export PGPASSWORD="${DB_PASSWORD}"

psql_cmd() {
    psql -v ON_ERROR_STOP=1 -h "${DB_HOST}" -p "${DB_PORT}" \
         -U "${DB_USER}" -d "${DB_NAME}" "$@"
}

log() {
    echo "[seed] $*"
}

log "destino: ${DB_USER}@${DB_HOST}:${DB_PORT}/${DB_NAME}"
log "directorio: ${SEEDS_DIR}"

shopt -s nullglob
found_any=0

for seed in "${SEEDS_DIR}"/[0-9]*.sql; do
    found_any=1
    name="$(basename "${seed}" .sql)"
    log "apply  ${name}"
    if ! psql_cmd -f "${seed}" >/dev/null; then
        log "ERROR  ${name} falló"
        exit 1
    fi
done

if [[ "${found_any}" == "0" ]]; then
    log "no hay seeds que aplicar"
    exit 0
fi

log "estado actual:"
psql_cmd -c "
SELECT reino, COUNT(*) AS especies
FROM especies
GROUP BY reino
ORDER BY reino;
"

log "OK"
