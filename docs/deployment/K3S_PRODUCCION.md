# Deploy k3s — Producción (VPS dedicado)

Guía operativa para desplegar la API en un **VPS Linux dedicado** con k3s. No
contiene secrets reales.

> **Alcance.** Este documento aplica a un VPS dedicado a este proyecto, donde
> k3s puede tomar los puertos 80/443 y ser el único runtime de contenedores.
>
> **No** describe el despliegue público que está corriendo hoy. Ese vive en un
> host compartido con otras cargas en producción y usa Docker Compose + un túnel
> Cloudflare propio: ver
> [PRODUCCION_DOCKER_CLOUDFLARE.md](PRODUCCION_DOCKER_CLOUDFLARE.md) y el ADR #13
> de [PLAN_MAESTRO.md §10](../PLAN_MAESTRO.md). Instalar k3s en aquel host
> pondría en riesgo un negocio ajeno a este proyecto.
>
> Los manifiestos de `infrastructure/kubernetes/` se mantienen para esta ruta.

## Prerrequisitos

- VPS Ubuntu/Debian con acceso SSH.
- Dominio apuntando al VPS, por ejemplo `api.chiloe-biodiversidad.example.com`.
- Puertos 80 y 443 abiertos.
- `kubectl` local configurado contra el cluster.
- `cert-manager` instalado.

## Instalación base en VPS

```bash
curl -sfL https://get.k3s.io | sh -
sudo cat /etc/rancher/k3s/k3s.yaml
```

Traefik viene activo por defecto en k3s. Si se desactiva, el overlay de
producción debe cambiar `ingressClassName`.

## Cert-manager

```bash
kubectl apply -f https://github.com/cert-manager/cert-manager/releases/download/v1.21.0/cert-manager.yaml
kubectl -n cert-manager rollout status deploy/cert-manager
kubectl -n cert-manager rollout status deploy/cert-manager-webhook
```

Antes de aplicar producción, reemplaza:

- `admin@example.com` en `infrastructure/kubernetes/overlays/production/cluster-issuer-letsencrypt-prod.yml`.
- `api.chiloe-biodiversidad.example.com` en `infrastructure/kubernetes/overlays/production/ingress-production.yml`.

## Secrets

No aplicar `secrets.production.template.yml` tal cual.

```bash
cp infrastructure/kubernetes/overlays/production/secrets.production.template.yml /tmp/chiloe-secrets.yml
$EDITOR /tmp/chiloe-secrets.yml
kubectl apply -f /tmp/chiloe-secrets.yml
```

## Deploy

```bash
kubectl kustomize --load-restrictor=LoadRestrictionsNone infrastructure/kubernetes/overlays/production | kubectl apply -f -
kubectl -n chiloe-flora rollout status deploy/postgres
kubectl -n chiloe-flora rollout status deploy/redis
kubectl -n chiloe-flora rollout status deploy/minio
kubectl -n chiloe-flora rollout status deploy/auth-service
kubectl -n chiloe-flora rollout status deploy/especies-api
kubectl -n chiloe-flora rollout status deploy/gateway
```

## Verificación

```bash
kubectl -n chiloe-flora get pods
kubectl -n chiloe-flora get ingress
curl -fsS https://api.chiloe-biodiversidad.example.com/health
curl -fsS https://api.chiloe-biodiversidad.example.com/docs-api.json
```

## Backups

Pendiente antes de producción real:

- `pg_dump` nocturno de PostgreSQL a almacenamiento externo.
- `mc mirror` nocturno de MinIO a B2/S3 externo.
- Prueba de restauración documentada.
