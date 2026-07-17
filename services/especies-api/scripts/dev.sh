#!/usr/bin/env bash
# Wrapper de desarrollo para especies-api.
# El compose canónico vive en la raíz del repo: infrastructure/docker/docker-compose.dev.yml.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

cd "${REPO_ROOT}"

if [[ "${1:-}" == "clean" ]]; then
    make dev-down-volumes
else
    make dev
    make logs-especies
fi
