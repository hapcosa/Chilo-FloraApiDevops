# Gateway = Nginx + el panel de curaduría compilado.
#
# El panel es un bundle estático: no hay servidor propio que mantener, así que
# viaja dentro de la misma imagen que ya sirve el reverse proxy. El contexto de
# build es la raíz del repo porque el código del panel vive en services/.

FROM node:20-alpine AS panel

# URL pública del object storage. El bundle es estático y no lee configuración
# en runtime, así que esto se fija aquí (ver docker-compose: se pasa como
# build-arg desde S3_PUBLIC_ENDPOINT).
ARG VITE_S3_PUBLIC_BASE=""
ENV VITE_S3_PUBLIC_BASE=$VITE_S3_PUBLIC_BASE

WORKDIR /panel

COPY services/panel-curaduria/package.json services/panel-curaduria/package-lock.json ./
RUN npm ci

COPY services/panel-curaduria/ ./
RUN npm run build


FROM nginx:1.25-alpine

# El nginx.conf lo monta el compose (dev y prod usan configuraciones distintas);
# aquí solo va el contenido estático.
COPY --from=panel /panel/dist/ /etc/nginx/html/curaduria/
COPY services/gateway/docs-api.json /etc/nginx/html/docs-api.json
