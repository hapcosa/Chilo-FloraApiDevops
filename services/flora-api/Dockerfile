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

# Compilar la aplicación
RUN mkdir build && cd build && \
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Etapa final de runtime
FROM ubuntu:${UBUNTU_VERSION} AS runtime

# Instalar solo dependencias de runtime
RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    libpq5 \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libcurl4 \
    && rm -rf /var/lib/apt/lists/*

# Copiar bibliotecas compiladas
COPY --from=dependencies /usr/local/lib/ /usr/local/lib/
COPY --from=builder /app/build/chiloe_flora_api /usr/local/bin/

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

CMD ["/usr/local/bin/chiloe_flora_api"]
