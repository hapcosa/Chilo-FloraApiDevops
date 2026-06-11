# AGENTS.md - Instrucciones para agentes de IA en este repo

Este archivo define como debe trabajar cualquier sesion con IA en este
proyecto. La prioridad es actuar como un agente profesional de programacion:
leer antes de tocar, razonar con evidencia, mantener seguridad, cuidar la
arquitectura, trabajar con DevOps real y entregar cambios probados.

Si una instruccion de este archivo entra en conflicto con una orden explicita
del humano, pregunta antes de actuar. Si entra en conflicto con una regla de
seguridad, gana la regla de seguridad.

---

## 1. Fuentes de verdad

Antes de proponer cambios estructurales, lee estas fuentes:

1. `docs/PLAN_MAESTRO.md`: plan vivo, arquitectura, fases y ADRs.
2. `CLAUDE.md`: guia heredada para asistentes de IA.
3. `README.MD`, `Makefile` y README de cada servicio.
4. Codigo y manifiestos reales del repo. El codigo manda sobre supuestos.

No inventes archivos, endpoints, variables de entorno, librerias ni flujos. Si
algo no esta verificado en el repo, dilo como hipotesis o pregunta.

---

## 2. Contexto del proyecto

Proyecto: plataforma de biodiversidad de Chiloe con backend de microservicios y
futura app Android offline-first.

Stack actual:

- `services/especies-api/`: C++17 + Pistache + libpqxx + PostgreSQL.
- `services/auth-service/`: Go 1.21 + Gin + GORM + JWT + Redis.
- `services/gateway/`: Nginx como API Gateway.
- Datos: PostgreSQL, Redis, migraciones SQL planas.
- Fotos: decision arquitectonica MinIO/S3 con presigned URLs.
- Infra: Docker Compose dev, Kubernetes, Terraform heredado.
- CI/CD: GitHub Actions, Docker builds, tests de C++ y Go.
- Mobile futuro: React Native bare + TypeScript + modulo nativo C++ con NDK
  Camera2 + SQLite offline.

Nombres historicos aceptados:

- Servicio nuevo: `especies-api`.
- Binario nuevo: `chiloe_especies_api`.
- DB/usuario/namespace heredados pueden seguir usando `chiloe_flora` y
  `flora_user` hasta que exista migracion coordinada.

---

## 3. Protocolo anti-alucinacion

Un agente debe:

- Revisar `git status --short` antes de editar.
- Buscar con `rg` o `rg --files` antes de asumir ubicaciones.
- Citar rutas reales cuando explique cambios.
- Separar hechos verificados de inferencias.
- Confirmar en codigo antes de afirmar que un endpoint, test o job existe.
- No depender de memoria sobre librerias, APIs externas o GitHub Actions si la
  informacion puede haber cambiado; consultar documentacion oficial cuando sea
  necesario.
- Si no puede ejecutar una prueba, decirlo claramente y explicar el riesgo.

Frases prohibidas en la practica:

- "Deberia funcionar" sin prueba o razon tecnica concreta.
- "Ya esta implementado" sin haber visto el archivo o el commit.
- "Es seguro" sin revisar entradas, secretos, permisos y efectos laterales.

---

## 4. Forma de trabajo profesional

Flujo normal:

1. Entender el objetivo y leer el contexto minimo.
2. Identificar superficie de cambio y riesgos.
3. Hacer cambios pequenos, coherentes con el estilo existente.
4. Agregar o ajustar tests cuando haya cambio de comportamiento.
5. Ejecutar las pruebas relevantes.
6. Resumir que cambio, como se verifico y que queda pendiente.

No hagas refactors grandes mezclados con features. No cambies arquitectura sin
actualizar `docs/PLAN_MAESTRO.md` y registrar una decision nueva en la seccion
de ADRs si corresponde.

---

## 5. DevOps, CI/CD y ramas

Regla de pipeline:

1. Trabajar en rama corta: `feat/...`, `fix/...`, `docs/...`, `test/...`,
   `chore/...`.
2. No hacer push directo a `master`.
3. Abrir PR contra `master`.
4. Esperar checks verdes.
5. Requiere revision humana antes de merge.
6. Deploy automatizado solo despues del merge.

Nunca usar sin permiso explicito:

- `git reset --hard`
- `git push --force`
- `git clean -fd`
- `--no-verify`
- borrado masivo de archivos

CI esperado:

- C++: build Docker, migraciones SQL, gtest/ctest.
- Go: `go test ./...` y build del binario.
- Integracion: docker compose dev/test cuando aplique.
- Mobile futuro: lint, Jest, build APK debug y e2e criticos.

Discrepancia conocida:

- El plan maestro apunta a k3s en VPS como destino natural de produccion.
- El workflow actual conserva partes AWS/EKS/ECR y algunas condiciones con
  `main` aunque el flujo documentado usa `master`.
- No "arreglar" esto de pasada. Si se corrige, hacerlo en PR dedicado y
  actualizar `docs/PLAN_MAESTRO.md`.

---

## 6. Arquitectura y limites entre servicios

`especies-api`:

- Es responsable del catalogo multi-reino, taxonomia, busqueda, filtros,
  validacion de `atributos_especificos` y referencias a fotos.
- No debe manejar usuarios, passwords, login ni sesiones.
- Valida datos de dominio antes de persistir.

`auth-service`:

- Es responsable de registro, login local, login Google, refresh tokens,
  perfiles y emision de JWT propio.
- Redis se usa para sesiones/refresh tokens.

`gateway`:

- Enruta `/auth/*` hacia `auth-service`.
- Enruta `/api/*` hacia `especies-api`.
- Debe concentrar controles transversales cuando aplique: TLS, rate limit,
  headers, auth_request/JWT.

Base de datos:

- Migraciones SQL numeradas en `services/<servicio>/migrations/`.
- No crear tablas desde codigo de aplicacion.
- No editar migraciones ya mergeadas; crear una nueva.
- Usar consultas parametrizadas, no concatenar SQL con input externo.

---

## 7. Modelo multi-reino

Decision vigente:

- Tabla base `especies` con columnas comunes.
- `reino_enum`: `animalia`, `plantae`, `fungi`, `protista`, `monera`.
- Campos especificos por reino en `atributos_especificos JSONB`.
- Validacion por JSON Schema en C++.

Reglas:

- Campo comun a todos los reinos: columna en `especies`.
- Campo especifico de un reino: propiedad en el schema JSON de ese reino.
- Fungi requiere especial cuidado por comestibilidad y riesgo sanitario.
- Monera requiere dominio cuando aplique segun schema.
- No ampliar a reinos no acordados sin ADR.

---

## 8. Fotos y object storage

Decision vigente:

- Usar MinIO/S3-compatible.
- La API no debe recibir uploads grandes `multipart/form-data` como flujo
  final de produccion.
- Flujo correcto: cliente pide presigned URL, sube directo a storage y luego
  informa la key a la API.

Reglas:

- Guardar en PostgreSQL solo keys o metadatos, nunca bytes de fotos.
- Validar que la key pertenece al bucket/prefijo esperado.
- Separar fotos curadas (`especies-fotos`) de avistamientos de usuarios
  (`avistamientos-fotos`).
- Eliminar EXIF sensible salvo consentimiento explicito del usuario.

---

## 9. Autenticacion y seguridad

Flujo objetivo:

- App Android obtiene `idToken` con Google Sign-In SDK.
- `auth-service` verifica el `idToken` contra Google.
- `auth-service` crea/asocia usuario y emite JWT propio.
- El cliente usa el JWT propio contra el backend.

Discrepancia conocida:

- En el codigo/config actual hay restos de flujo OAuth web con
  `GOOGLE_CLIENT_SECRET` y redirect URL.
- El plan maestro exige flujo mobile por `idToken`.
- No mezclar ambos sin documentar decision y tests.

Reglas de seguridad:

- No commitear secretos, `.env` reales, kubeconfigs, tokens ni claves privadas.
- Passwords siempre hasheadas con algoritmo fuerte; nunca texto plano.
- JWT con expiracion, issuer/audience cuando aplique y secret desde entorno.
- Validar input en handlers y services.
- No filtrar detalles internos en errores publicos.
- Aplicar principio de minimo privilegio en K8s, buckets, DB y CI secrets.
- Revisar CORS, rate limits y headers de seguridad al tocar gateway.

---

## 10. Testing y definicion de listo

Un cambio esta listo cuando:

- Compila.
- Tiene tests nuevos o existentes adecuados al riesgo.
- Las pruebas relevantes pasan localmente o en CI.
- No introduce secretos ni dependencias innecesarias.
- Respeta el plan maestro o lo actualiza.
- El resumen final incluye verificacion real.

Comandos utiles:

```bash
make test
make go-test
make cpp-build
docker build --target tester -t especies-api-test services/especies-api
cd services/auth-service && go test ./...
```

Si un comando falla por entorno local, investigar primero. Si sigue fallando por
dependencias externas, reportar el bloqueo con salida relevante.

---

## 11. Practicas agiles

Trabajar en incrementos pequenos:

- Una fase o slice por PR.
- Criterios de aceptacion claros.
- Cambios observables y testeables.
- Feedback rapido por CI.
- Documentar decisiones, no solo codigo.

Para cada tarea, responder:

- Que problema resuelve.
- Que archivos toca.
- Que pruebas lo cubren.
- Que riesgo residual queda.

---

## 12. Estilo de comunicacion del agente

El agente debe ser directo, tecnico y honesto:

- Avisar que esta leyendo antes de editar.
- Pedir confirmacion solo cuando la decision sea riesgosa o no verificable.
- No vender humo ni sobredimensionar el trabajo.
- Explicar supuestos.
- Dejar handoff claro para la siguiente sesion.

Idioma:

- Conversacion, documentacion y mensajes publicos: espanol.
- Identificadores de codigo: seguir estilo existente.
- Errores tecnicos internos pueden ir en ingles si el servicio ya lo usa.

---

## 13. Checklist de inicio para cada sesion

1. Leer el pedido del humano completo.
2. Ejecutar o revisar `git status --short`.
3. Leer `docs/PLAN_MAESTRO.md` si la tarea toca arquitectura o fases.
4. Buscar archivos relevantes con `rg`.
5. Detectar cambios no propios y no revertirlos.
6. Planear el cambio minimo.
7. Editar.
8. Probar.
9. Resumir con rutas y resultados.

---

## 14. Checklist antes de cerrar una respuesta

- Quedo el cambio hecho, no solo propuesto?
- Se respetaron las reglas de seguridad?
- Se ejecutaron pruebas relevantes?
- Se menciono cualquier prueba no ejecutada?
- Se evitaron cambios fuera de alcance?
- Se dejo claro el siguiente paso del plan maestro?
