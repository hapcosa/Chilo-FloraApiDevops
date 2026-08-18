# Prompt: arrancar la Fase 9 en otra sesión

Pégalo tal cual al abrir la sesión nueva. **Un agente = un worktree**: si hay
otra sesión trabajando este repo —por ejemplo la de la cámara, ver
[PROMPT_CAMARA_SESION.md](PROMPT_CAMARA_SESION.md)— esta va en su propio
`git worktree`, nunca en el mismo directorio.

---

## Qué hay que hacer

Ejecutar el plan de [docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md](docs/PLAN_FASE_9_TURISMO_Y_COMUNIDAD.md),
**en orden**, un PR por punto. El plan ya está acordado con el usuario: no hay
que rediseñarlo, hay que construirlo. Si algo del plan resulta equivocado al
tocarlo de verdad, decirlo y proponer el cambio en el mismo PR que lo corrige.

Son 13 PRs en cuatro fases. El orden importa: la Fase 9.0 es barata y visible,
y varias cosas de después dependen de ella.

| Fase | PRs | De qué va |
|---|---|---|
| 9.0 | 1–4 | Quitar emojis de las fichas, ofrecer crear encuentro tras la captura, contar encuentros en vez de fichas abiertas, advertencia de fauna |
| 9.1 | 5–7 | Bio y profesión en el perfil, editar perfil de verdad, encuentros anteriores a la app |
| 9.2 | 8–10 | Mapa satelital de encuentros, endpoint agregado, parques de Chiloé |
| 9.3 | 11–13 | Insignias, pantalla de usuarios del panel, **postular a curar** |

**Empezar por el PR 1**, que es el más chico y sirve para verificar que el ciclo
completo (rama → CI → PR) funciona en esta sesión.

---

## Contexto que no está en el código

**Producción está viva y el usuario la usa.** El panel de curaduría responde en
`https://api.budaicapital.com/curaduria/`, ya tiene login con Google, y la
cuenta `hapcosa@gmail.com` (id 4) es `admin`.

**Mergear no despliega.** Hay que reconstruir a mano en el host de prod
(`donaldchavez@10.244.117.161`, checkout en
`~/servicios/chiloe-biodiversidad-api`), nombrando **siempre el servicio
concreto**:

```bash
cd ~/servicios/chiloe-biodiversidad-api/infrastructure/docker
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml build gateway
docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env \
  -f docker-compose.prod.yml up -d gateway
```

⚠️ Nunca correr dos `cloudflared` del mismo túnel a la vez: Cloudflare ve dos
conectores y reparte el tráfico. Por eso jamás un `build`/`up` sin argumento.

Las credenciales SSH están en `~/.env` (`SSH_PROD_*`), se usan con `sshpass -e`
y **nunca** se imprimen. El panel se compila **dentro de la imagen del
gateway**: no es un servicio con puerto propio.

**El JDK por defecto del host es openjdk 26.0.2 y el plugin Gradle de React
Native no parsea esa versión.** Para cualquier build de Android:

```bash
cd mobile/android && JAVA_HOME=/usr/lib/jvm/java-17-openjdk ./gradlew assembleRelease
```

No meter esa ruta en el repo.

---

## Dos huecos que el plan ya recoge y conviene tener presentes

1. **Nadie puede postular a curar.** `POST /api/v1/postulaciones` y la bandeja
   de revisión del panel existen desde la migración `0005`, pero ningún cliente
   tiene interfaz para postular: `mobile/src/` no menciona postulaciones en
   ningún archivo. Es el **PR 13**.
2. **Un admin no puede repartir permisos por la web.** Los endpoints
   `POST`/`DELETE /api/v1/categorias/:id/moderadores/:usuarioId` existen y son
   admin-only, pero ninguna pantalla los llama y no hay listado de usuarios. Es
   el **PR 12**, que además tiene una duda abierta sin resolver: `users` vive en
   `auth-service` y las asignaciones en `especies-api`, así que hay que decidir
   si el panel consulta a los dos o si uno expone la vista combinada. **Esa
   decisión se toma con el usuario antes de escribir código.**

---

## Decisiones ya tomadas — no reabrirlas

- **Mapa**: `react-native-maps` con proveedor Google, satelital. Es la única
  dependencia de peso que el plan autoriza; su key va en el
  `AndroidManifest.xml` del APK (Maps SDK for Android), **no** en el env del
  backend, y va **restringida por nombre de paquete y huella SHA-1** de firma.
- **Sin ranking público de encuentros.** Premiar el volumen empuja exactamente
  la conducta que la advertencia de fauna intenta desalentar. Las insignias
  reconocen constancia y variedad, sin tabla de posiciones. Si el usuario
  insiste, es su decisión, pero hay que decírselo antes.
- **Fungi conserva su marca de advertencia** de comestibilidad aunque el PR 1
  quite los demás emojis. Es riesgo sanitario real.
- **La ubicación es el dato más sensible de la app.** El mapa comunitario, los
  encuentros retroactivos y las insignias empujan todos hacia publicar más
  ubicaciones y más precisas. La ofuscación de coordenadas del PR 9 y la
  ausencia de ranking son contrapesos deliberados, no adornos.

---

## Reglas innegociables

- Rama por cambio → commits → push → **PR contra `master`** → checks verdes →
  **el merge lo hace el usuario**. Nada de push a `master`, `--force`,
  `reset --hard` ni `--no-verify`.
- **Migraciones numeradas** en `services/<servicio>/migrations/`, una por PR
  cuando se pueda. **Nunca** se editan tras mergear: si hay que corregir, va una
  migración nueva. Las tablas no se crean desde código de aplicación.
- **Sin dependencias nuevas** sin justificarlas en el PR.
- **Fotos**: nunca multipart contra la API. Presigned URL → subida directa →
  notificar la key. EXIF sensible (GPS, serial) fuera salvo opt-in explícito.
- TypeScript estricto, ESLint, Jest en mobile y panel; `gofmt` + `golangci-lint`
  en Go; gtest y estilo existente en C++.
- Español en documentación, UI y comentarios; inglés en identificadores, salvo
  los nombres del dominio biológico (`reino`, `especie`, `avistamiento`).
- **Avisar toda desviación**: cualquier default elegido, aproximación tomada o
  parte del pedido omitida se lista con su razón. Nunca en silencio.
- No commitear: `.idea/`, `diseño/`, keystores, `google-services.json`, claves
  de firma, kubeconfig ni `.env` reales.
- Si algo falla, se dice con el output. Si un paso se salteó, se dice.

---

## Comandos

```bash
make dev / make test / make health-check     # backend
cd mobile && npm run typecheck && npm run lint && npm test
cd services/panel-curaduria && npm run lint && npm run build && npm test
```

`especies-api` **solo compila dentro de Docker**: al host le faltan Pistache y
libpqxx, así que un `cmake` directo falla.
