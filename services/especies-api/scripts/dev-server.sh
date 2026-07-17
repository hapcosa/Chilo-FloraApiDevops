#!/usr/bin/env bash
# Hot reload para especies-api dentro del contenedor de desarrollo.

set -euo pipefail

APP_PID=""

log() {
    echo "[especies-api-dev] $*"
}

build_app() {
    log "building application"
    mkdir -p /app/build
    cd /app/build

    rm -f CMakeCache.txt

    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}" ..
    make -j"$(nproc)"
}

run_app() {
    if [[ ! -x "/app/build/chiloe_especies_api" ]]; then
        log "executable not found: /app/build/chiloe_especies_api"
        return 1
    fi

    log "starting application"
    cd /app
    ./build/chiloe_especies_api &
    APP_PID="$!"
    log "application pid: ${APP_PID}"
}

stop_app() {
    if [[ -n "${APP_PID}" ]] && kill -0 "${APP_PID}" 2>/dev/null; then
        log "stopping application pid ${APP_PID}"
        kill "${APP_PID}"
        wait "${APP_PID}" 2>/dev/null || true
    fi
}

cleanup() {
    stop_app
    exit 0
}

trap cleanup SIGTERM SIGINT

log "starting development server"
log "schemas dir: ${SCHEMAS_DIR:-not set}"

if build_app; then
    run_app || true
else
    log "initial build failed; waiting for file changes"
fi

log "watching /app/src, /app/include and /app/CMakeLists.txt"
while inotifywait -e modify,create,delete,move -r /app/src /app/include /app/CMakeLists.txt; do
    stop_app
    sleep 1

    if build_app; then
        run_app || true
    else
        log "build failed; waiting for next change"
    fi
done
