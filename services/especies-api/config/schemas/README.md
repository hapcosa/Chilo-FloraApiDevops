# JSON Schemas por reino

Estos schemas validan el campo `atributos_especificos JSONB` de la tabla `especies` en función del `reino`. La validación ocurre en C++ con `nlohmann/json-schema-validator` antes de cualquier INSERT/UPDATE.

## Layout

```
config/schemas/
├── animalia.json
├── plantae.json
├── fungi.json
├── protista.json
├── monera.json
└── README.md
```

El validador (`AtributosSchemaValidator`) se construye en `main.cpp` con la ruta a esta carpeta y carga los 5 schemas al arrancar. Si un archivo falta o no parsea, el servicio aborta el arranque — no se permite estado degradado.

## Convenciones

- Draft 2020-12 (`$schema` declarado en cada archivo).
- `additionalProperties: false` en todos los schemas y subobjetos: campos desconocidos hacen fallar la validación. Esto fuerza coherencia con el esquema; si quieres añadir un campo nuevo a un reino, actualiza el schema en un PR.
- `required` sólo donde es crítico para divulgación:
  - **Fungi**: `comestibilidad` es obligatoria (riesgo sanitario).
  - **Monera**: `dominio` es obligatorio (bacteria/archaea son clados muy distintos).
  - El resto de reinos no tiene campos obligatorios.
- Strings sin acentos en los `enum` (e.g. `toxico` no `tóxico`, `otono` no `otoño`). Razones: comparaciones más estables entre clientes, evita issues de encoding.
- Campos de texto libre con `maxLength` razonable (~1000–3000 caracteres).
- Geolocalización, fotos y campos comunes (descripción, hábitat) **no** van aquí — son columnas de la tabla `especies`.

## Cómo añadir un campo

1. Editar el schema correspondiente.
2. Añadir test funcional que cubra el nuevo campo (válido / inválido).
3. PR con `feat(schema): añadir <campo> a <reino>`.

## Cómo añadir un reino

Habría que también:
1. Extender `reino_enum` en una migración SQL nueva.
2. Extender `enum class Reino` en `include/models/reino.hpp`.
3. Crear el nuevo archivo de schema.
4. Registrar el archivo en `AtributosSchemaValidator::loadAll()`.

Esto **no se permite sin discusión previa**: el plan maestro define 5 reinos. Cambiarlo requiere un ADR nuevo en `docs/PLAN_MAESTRO.md` §10.
