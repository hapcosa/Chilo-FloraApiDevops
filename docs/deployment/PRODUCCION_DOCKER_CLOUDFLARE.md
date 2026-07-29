# Producción sobre Docker Compose + Cloudflare Tunnel

Guía operativa del despliegue público actual: `https://api.budaicapital.com`
(API) y `https://storage.budaicapital.com` (object storage).

Este documento describe el despliegue **en un host compartido**. Para un VPS
dedicado con Kubernetes, ver [K3S_PRODUCCION.md](K3S_PRODUCCION.md); son dos
destinos distintos, no dos versiones del mismo.

No contiene secretos reales.

---

## 1. Por qué Docker Compose aquí y no k3s

El `PLAN_MAESTRO` elige k3s para el VPS de producción (ADR #6) y esa decisión
sigue en pie para un host dedicado. Este host concreto es otra cosa:

- Ya sostiene ~20 contenedores de otro negocio en producción real
  (`signalstrading-v2`, `kryptolab`, varias BD `obrero_db_*`).
- El Traefik que k3s instala por defecto quiere los puertos 80 y 443. En este
  host **nada** escucha ahí: la salida a internet es por Cloudflare Tunnel.
- El containerd de k3s conviviría mal con el Docker que ya corre esas cargas.

Instalar k3s aquí pondría en riesgo un negocio ajeno a este proyecto para ganar
paridad con unos manifiestos que de todas formas no se usan en este host. Los
manifiestos de `infrastructure/kubernetes/` siguen siendo válidos y son el
camino para migrar a un VPS dedicado más adelante.

Registrado como ADR #13 en [PLAN_MAESTRO.md §10](../PLAN_MAESTRO.md).

---

## 2. Arquitectura

```
internet → Cloudflare edge (TLS) → cloudflared (chiloe-cloudflared)
                                        │  red chiloe-prod-network
        api.budaicapital.com  ──────────┴──→ gateway (nginx) ──→ especies-api (C++)
                                                              └─→ auth-service (Go)
                                                                     │
                                                          postgres ──┴── redis
        storage.budaicapital.com ───────────────────────→ minio
```

Puntos que no son obvios:

- **MinIO necesita hostname propio.** El cliente pide una URL presignada a la
  API y después sube el archivo **directo a MinIO**. La foto nunca pasa por el
  gateway, así que el teléfono tiene que poder resolver y alcanzar MinIO.
- **Las presigned URLs se firman con la URL pública.** `especies-api` firma con
  `S3_PUBLIC_ENDPOINT` (`upload_presign_service.cpp` usa `publicEndpoint` para
  el PUT presignado y `endpoint` para el HEAD interno de verificación). MinIO
  valida la firma contra el `Host` que recibe, por eso además lleva
  `MINIO_SERVER_URL` con el mismo hostname. Si esto se configura mal, la API
  devuelve URLs apuntando a `minio:9000` y el teléfono no sube nada.
- **Ningún puerto se publica al host.** Todo se habla por la red Docker
  `chiloe-prod-network` (subred `172.28.0.0/16`, elegida por estar libre: las
  redes existentes usan 172.17/18/19/23).

---

## 3. Convivencia con el resto del host

Reglas que este stack respeta y que hay que mantener:

| Regla | Cómo se cumple |
|-------|----------------|
| No tocar `signalstrading-v2`, `kryptolab`, `obrero_db_*` | Proyecto Compose propio (`-p chiloe-prod`), red propia, volúmenes propios |
| No ocupar puertos del host | Ningún `ports:` en `docker-compose.prod.yml` |
| No colisionar de subred | `172.28.0.0/16` fija y explícita |
| No tocar el túnel `cyber-api` | Túnel propio `chiloe-api`, creado por CLI, con sus credenciales |
| No comerse la RAM | `mem_limit` en todos los servicios (~2,3 GB en total) |

---

## 4. Secretos

**Nada de esto va a git.** El archivo real vive fuera del árbol del repo:

```
/home/<usuario>/.config/chiloe-prod/chiloe.env      (chmod 600, dir chmod 700)
```

La plantilla versionada es
[`infrastructure/docker/.env.prod.example`](../../infrastructure/docker/.env.prod.example).

Credenciales generadas **nuevas** para este despliegue (no se reutilizó ninguna
de desarrollo):

- `JWT_SECRET` — `openssl rand -hex 32`
- `DB_PASSWORD` — `openssl rand -base64 24`
- `MINIO_ROOT_USER` / `MINIO_ROOT_PASSWORD` — aleatorias

Las credenciales del túnel las genera `cloudflared tunnel create` en
`~/.cloudflared/<uuid>.json`. Ese archivo tampoco se commitea: se monta en el
contenedor por ruta absoluta vía `CLOUDFLARED_CREDENTIALS_FILE`.

El `.gitignore` del repo ya ignora `.env`. `.env.prod.example` sí se versiona
porque solo tiene claves vacías.

---

## 5. Puesta en marcha desde cero

### 5.1 Túnel

```bash
cloudflared tunnel create chiloe-api
# → escribe ~/.cloudflared/<uuid>.json y muestra el UUID
```

Poner ese UUID en `infrastructure/docker/cloudflared/config.yml` y la ruta del
JSON en `CLOUDFLARED_CREDENTIALS_FILE`.

### 5.2 DNS

```bash
cloudflared tunnel route dns chiloe-api api.budaicapital.com
cloudflared tunnel route dns chiloe-api storage.budaicapital.com
```

> **En este host esto falla.** El `~/.cloudflared/cert.pem` de la máquina
> contiene solo un bloque `ARGO TUNNEL TOKEN`, sin certificado de zona: sirve
> para crear túneles (operación de cuenta) pero no lleva permiso de edición DNS
> sobre `budaicapital.com`, y `route dns` responde `code: 10000, Authentication
> error`. Los dos registros hay que crearlos desde el dashboard de Cloudflare, o
> con un API token que tenga `Zone:DNS:Edit`:
>
> ```
> api.budaicapital.com      CNAME  <uuid>.cfargotunnel.com   (proxied)
> storage.budaicapital.com  CNAME  <uuid>.cfargotunnel.com   (proxied)
> ```

### 5.3 Entorno

```bash
mkdir -p ~/.config/chiloe-prod && chmod 700 ~/.config/chiloe-prod
cp infrastructure/docker/.env.prod.example ~/.config/chiloe-prod/chiloe.env
chmod 600 ~/.config/chiloe-prod/chiloe.env
$EDITOR ~/.config/chiloe-prod/chiloe.env    # rellenar todos los secretos
```

Las variables críticas llevan `${VAR:?}` en el compose: si falta alguna, el `up`
falla en vez de arrancar Postgres con una contraseña por defecto.

### 5.4 Levantar

```bash
cd <raíz del repo>
docker compose -p chiloe-prod \
  --env-file ~/.config/chiloe-prod/chiloe.env \
  -f infrastructure/docker/docker-compose.prod.yml \
  up -d --build
```

Orden que impone el compose: `postgres`/`redis`/`minio` sanos →
`minio-create-buckets` y `especies-api-migrate` terminan con éxito →
`especies-api` y `auth-service` sanos → `gateway` sano → `cloudflared`.

`especies-api-migrate` aplica `services/especies-api/migrations/*.sql` con
tracking en `schema_migrations`; es idempotente y se puede relanzar.

La tabla de usuarios la crea `auth-service` al arrancar (GORM `AutoMigrate`), no
las migraciones SQL.

---

## 6. Buckets

`minio-create-buckets` los crea con estas políticas:

| Bucket | Política anónima |
|--------|------------------|
| `especies-fotos` | `download` |
| `avistamientos-fotos` | `none` |
| `perfiles-fotos` | `download` |

---

## 7. Diferencias respecto a `nginx.dev.conf`

`services/gateway/nginx.prod.conf` deriva de la config de dev. Lo que cambia y
por qué:

- Upstream `especies-api:9080` (puerto de la imagen de runtime), no `:9081`.
- Logs a `stdout`/`stderr` para `docker logs`.
- `set_real_ip_from 172.28.0.0/16` + `real_ip_header CF-Connecting-IP`. Sin esto
  todo el tráfico llega con la IP del contenedor `cloudflared` y el rate
  limiting no distingue clientes.
- **`/api/v1/uploads` exige sesión.** En dev es público y
  `upload_controller.cpp` no comprueba identidad: expuesto a internet sería un
  endpoint anónimo para escribir en los buckets. Es la única diferencia de
  modelo de autorización entre dev y prod, y conviene cerrarla también en dev.
- Sin `/nginx_status`.
- `client_max_body_size 1m`: las fotos no pasan por aquí.

---

## 8. Operación

```bash
# alias cómodo
alias chiloe-prod='docker compose -p chiloe-prod --env-file ~/.config/chiloe-prod/chiloe.env -f infrastructure/docker/docker-compose.prod.yml'

chiloe-prod ps
chiloe-prod logs -f especies-api
chiloe-prod restart gateway
chiloe-prod down                 # NO borra volúmenes
chiloe-prod up -d --build        # redeploy tras cambios de código
```

Shell a Postgres:

```bash
docker exec -it chiloe-postgres psql -U "$DB_USER" -d "$DB_NAME"
```

Administrar MinIO (la consola web está desactivada, `MINIO_BROWSER=off`):

```bash
docker run --rm -it --network chiloe-prod_chiloe-prod-network minio/mc:latest \
  sh -c 'mc alias set p http://minio:9000 <user> <pass> && mc ls p'
```

---

## 9. Verificación

Desde fuera del host:

```bash
curl -s https://api.budaicapital.com/health

# registro + login
curl -s -X POST https://api.budaicapital.com/api/v1/auth/register \
  -H 'Content-Type: application/json' \
  -d '{"email":"...","password":"...","nombre":"..."}'

TOKEN=$(curl -s -X POST https://api.budaicapital.com/api/v1/auth/login \
  -H 'Content-Type: application/json' \
  -d '{"email":"...","password":"..."}' | python3 -c 'import sys,json;print(json.load(sys.stdin)["token"])')

# especies: requiere sesión (auth_request, ADR #10). Sin token → 401.
curl -s -H "Authorization: Bearer $TOKEN" \
  "https://api.budaicapital.com/api/v1/especies?limit=3"

# ciclo de foto completo
curl -s -X POST https://api.budaicapital.com/api/v1/uploads/presign \
  -H "Authorization: Bearer $TOKEN" -H 'Content-Type: application/json' \
  -d '{"bucket":"avistamientos-fotos","filename":"prueba.jpg","content_type":"image/jpeg"}'
# → PUT del archivo a la url devuelta, con el header Content-Type que indica
```

Estado interno:

```bash
docker exec chiloe-cloudflared curl -s localhost:2000/ready
chiloe-prod ps            # ninguno debe estar reiniciándose
```

---

## 10. Pendientes

- **Seeds.** No existen en el repo: ni `services/especies-api/seeds/` ni
  `scripts/seed.sh`, y tampoco hay rama remota con ellos. La BD arranca vacía y
  la app se verá sin contenido hasta que se carguen especies.
- **Backups.** Igual que en k3s: falta `pg_dump` nocturno del volumen
  `chiloe-prod_postgres_data` y `mc mirror` de MinIO a almacenamiento externo,
  más una prueba de restauración.
- **App móvil.** `appConfig.apiBaseUrl` apunta a `http://localhost:8080`. Para
  la APK de producción hay que apuntarlo a `https://api.budaicapital.com`.
- **`/api/v1/uploads` en dev.** Sigue sin exigir sesión (ver §7).
- **Observabilidad.** Este stack no levanta Prometheus ni Grafana.
