BEGIN;

-- Un "encuentro" personal puede registrarse sin foto (el usuario marca que
-- vio la especie, con nota y ubicación, pero no siempre tiene o quiere
-- subir una imagen). foto_key deja de ser obligatorio a nivel de columna;
-- la aplicación sigue validando que, si viene, la key exista en el bucket.
ALTER TABLE avistamientos ALTER COLUMN foto_key DROP NOT NULL;

-- El constraint original exigía foto_key no-vacío siempre; ahora solo
-- aplica cuando el campo no es NULL.
ALTER TABLE avistamientos DROP CONSTRAINT IF EXISTS avistamientos_foto_key_not_blank;
ALTER TABLE avistamientos ADD CONSTRAINT avistamientos_foto_key_not_blank
    CHECK (foto_key IS NULL OR length(trim(foto_key)) > 0);

COMMIT;
