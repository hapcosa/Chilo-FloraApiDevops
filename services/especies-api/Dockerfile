ARG UBUNTU_VERSION=22.04
FROM ubuntu:${UBUNTU_VERSION} AS base

# Configurar timezone y locale
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=America/Santiago
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Instalar dependencias del sistema en una sola capa
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    meson \
    ninja-build \
    git \
    pkg-config \
    libpq-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    libboost-all-dev \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Etapa para compilar dependencias
FROM base AS dependencies

# Instalar nlohmann/json
RUN mkdir -p /usr/include/nlohmann && \
    curl -L https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp \
    -o /usr/include/nlohmann/json.hpp

# Compilar nlohmann/json-schema-validator (pboettch).
# Valida atributos_especificos JSONB por reino antes de INSERT/UPDATE.
# Versión fijada por reproducibilidad.
RUN git clone --depth=1 --branch=2.3.0 https://github.com/pboettch/json-schema-validator.git /tmp/json-schema-validator && \
    cd /tmp/json-schema-validator && \
    mkdir build && cd build && \
    cmake -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DJSON_VALIDATOR_BUILD_TESTS=OFF \
        -DJSON_VALIDATOR_BUILD_EXAMPLES=OFF \
        -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j$(nproc) && make install && \
    rm -rf /tmp/json-schema-validator

# Compilar Pistache
RUN git clone --depth=1 --branch=master https://github.com/pistacheio/pistache.git /tmp/pistache && \
    cd /tmp/pistache && \
    meson setup build --buildtype=release --prefix=/usr/local && \
    cd build && ninja && ninja install && \
    rm -rf /tmp/pistache

# Compilar libpqxx
RUN git clone --depth=1 --branch=7.8.1 https://github.com/jtv/libpqxx.git /tmp/libpqxx && \
    cd /tmp/libpqxx && \
    mkdir build && cd build && \
    cmake -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DSKIP_BUILD_TEST=ON \
        -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j$(nproc) && make install && \
    rm -rf /tmp/libpqxx

# Compilar bcrypt
RUN git clone --depth=1 https://github.com/trusch/libbcrypt.git /tmp/libbcrypt && \
    cd /tmp/libbcrypt && \
    cmake . && make && make install && \
    rm -rf /tmp/libbcrypt

# Compilar jwt-cpp
RUN git clone --depth=1 https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp && \
    cd /tmp/jwt-cpp && \
    mkdir build && cd build && \
    cmake .. && make && make install && \
    rm -rf /tmp/jwt-cpp

# Actualizar cache de bibliotecas
RUN ldconfig

# Etapa de compilación de la aplicación
FROM dependencies AS builder

WORKDIR /app

# Copiar archivos necesarios para la compilación (sin ../ porque el contexto ya está en el directorio padre)
COPY CMakeLists.txt ./
COPY src ./src/
COPY include ./include/
COPY config ./config/

# Compilar la aplicación
RUN mkdir build && cd build && \
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Etapa de tests (gtest). Reutiliza `dependencies` para compartir Pistache,
# libpqxx y json-schema-validator. Se ejecuta en CI con:
#   docker build --target tester -t especies-api-test services/especies-api
# El build falla si algún test falla (ctest --output-on-failure).
FROM dependencies AS tester

RUN apt-get update && apt-get install -y --no-install-recommends \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY CMakeLists.txt ./
COPY src ./src/
COPY include ./include/
COPY config ./config/
COPY tests ./tests/

RUN mkdir build && cd build && \
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON .. && \
    make -j$(nproc) unit_tests && \
    ctest --output-on-failure

# Etapa final de runtime
FROM ubuntu:${UBUNTU_VERSION} AS runtime

# Instalar solo dependencias de runtime
RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    libpq5 \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libcurl4 \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Copiar bibliotecas compiladas y schemas
COPY --from=dependencies /usr/local/lib/ /usr/local/lib/
COPY --from=builder /app/build/chiloe_especies_api /usr/local/bin/
COPY --from=builder /app/config/schemas /etc/chiloe-especies-api/schemas

# Actualizar cache de bibliotecas
RUN ldconfig

# Crear usuario no-root
RUN groupadd -r appuser && useradd -r -g appuser appuser
USER appuser

# Configurar variables de entorno
ENV API_PORT=9080
ENV API_HOST=0.0.0.0

EXPOSE ${API_PORT}

HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:${API_PORT}/health || exit 1

CMD ["/usr/local/bin/chiloe_especies_api"]
