-- =============================================================================
-- 0001_initial.sql
-- =============================================================================
-- Baseline del schema de especies-api, extraído de los CREATE TABLE
-- IF NOT EXISTS que vivían embebidos en los repositorios C++.
--
-- Esta migración refleja el estado del schema antes de la Fase 1 multi-reino.
-- La siguiente migración (0002_multi_reino.sql) introduce reino_enum,
-- atributos_especificos JSONB y el modelo de fotos basado en object storage.
-- =============================================================================

BEGIN;

-- -----------------------------------------------------------------------------
-- familias
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS familias (
    id          SERIAL PRIMARY KEY,
    nombre      VARCHAR(100) NOT NULL UNIQUE,
    descripcion TEXT
);

CREATE TABLE IF NOT EXISTS familia_imagenes (
    id           SERIAL PRIMARY KEY,
    familia_id   INTEGER REFERENCES familias(id) ON DELETE CASCADE,
    url          VARCHAR(255) NOT NULL,
    es_principal BOOLEAN DEFAULT FALSE,
    UNIQUE (familia_id, url)
);

-- -----------------------------------------------------------------------------
-- generos
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS generos (
    id          SERIAL PRIMARY KEY,
    nombre      VARCHAR(100) NOT NULL UNIQUE,
    descripcion TEXT,
    familia_id  INTEGER REFERENCES familias(id)
);

CREATE TABLE IF NOT EXISTS genero_imagenes (
    id           SERIAL PRIMARY KEY,
    genero_id    INTEGER REFERENCES generos(id) ON DELETE CASCADE,
    url          VARCHAR(255) NOT NULL,
    es_principal BOOLEAN DEFAULT FALSE,
    UNIQUE (genero_id, url)
);

-- -----------------------------------------------------------------------------
-- especies
-- -----------------------------------------------------------------------------
CREATE TABLE IF NOT EXISTS especies (
    id                  SERIAL PRIMARY KEY,
    nombre_cientifico   VARCHAR(100) NOT NULL UNIQUE,
    nombre_comun        VARCHAR(100),
    descripcion         TEXT,
    habitat             TEXT,
    distribucion        TEXT,
    endemica            BOOLEAN DEFAULT FALSE,
    genero_id           INTEGER REFERENCES generos(id),
    estado_conservacion VARCHAR(50)
);

CREATE TABLE IF NOT EXISTS especies_imagenes (
    id           SERIAL PRIMARY KEY,
    especie_id   INTEGER REFERENCES especies(id) ON DELETE CASCADE,
    url          VARCHAR(255) NOT NULL,
    es_principal BOOLEAN DEFAULT FALSE,
    UNIQUE (especie_id, url)
);

COMMIT;
