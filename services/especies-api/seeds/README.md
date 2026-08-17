# Seeds — contenido inicial de especies-api

Datos de contenido (no de esquema). Se aplican con `../scripts/seed.sh` **después**
de las migraciones.

## Diferencia con `migrations/`

| | migrations | seeds |
|---|---|---|
| Qué contienen | DDL: tablas, índices, tipos | Filas de contenido divulgativo |
| Tracking | tabla `schema_migrations` | ninguno |
| Reaplicar | se salta si ya se aplicó | se reaplica siempre, sin duplicar |
| Editar tras mergear | **nunca** | sí, con las reglas de abajo |

Los seeds son idempotentes por construcción: cada `INSERT` lleva
`ON CONFLICT ... DO NOTHING` sobre la clave única natural
(`especies.nombre_cientifico`, `familias(reino, nombre)`,
`generos(familia_id, nombre)`). Por eso no hace falta tabla de tracking y por eso
correr `seed.sh` dos veces deja la BD igual que correrlo una.

## Reglas al editar

- **Corregir** el texto de una especie ya mergeada: se edita el seed, pero el
  cambio **no** se propaga a las BD donde ya se insertó la fila (el
  `ON CONFLICT DO NOTHING` la deja intacta). Si la corrección debe llegar a
  producción, va en una migración de datos aparte.
- **Añadir** especies: preferible un archivo nuevo (`0002_*.sql`) antes que
  crecer el existente.
- Los `atributos_especificos` deben validar contra el JSON Schema del reino en
  [`../config/schemas/`](../config/schemas/). Todos usan
  `additionalProperties: false`: un campo de más hace fallar la validación en la
  API, no en el `INSERT`.

## Uso

```bash
cd services/especies-api
DB_NAME=chiloe_flora_dev DB_USER=dev_user DB_PASSWORD=dev_password ./scripts/seed.sh
```

## Archivos

| Archivo | Contenido |
|---------|-----------|
| `0001_especies_chiloe.sql` | 13 familias, 13 géneros y 13 especies del archipiélago, cubriendo los cinco reinos |
| `0002_especies_chiloe_ampliado.sql` | 66 familias, 84 géneros y 90 especies más (49 animalia, 30 plantae, 6 fungi, 3 protista, 2 monera), con fichas largas y `atributos_especificos` completos |

Aplicando ambos: **103 especies** (53 animalia, 34 plantae, 8 fungi, 5 protista,
3 monera).

⚠️ Las fichas de `0002_*` se redactaron a partir de conocimiento general de la
biota chilota, **no** transcribiendo una fuente por especie. Pesos, tamaños,
épocas reproductivas y estados de conservación son plausibles pero no están
verificados uno a uno: revisar antes de tratarlos como dato duro.
