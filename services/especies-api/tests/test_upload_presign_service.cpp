#include <gtest/gtest.h>

#include <stdexcept>

#include "services/upload_presign_service.hpp"

namespace {

UploadStorageConfig testConfig() {
    UploadStorageConfig config;
    config.endpoint = "http://minio:9000";
    config.publicEndpoint = "http://localhost:9000";
    config.region = "us-east-1";
    config.accessKeyId = "minioadmin";
    config.secretAccessKey = "minioadmin123";
    config.especiesBucket = "especies-fotos";
    config.avistamientosBucket = "avistamientos-fotos";
    config.perfilesBucket = "perfiles-fotos";
    config.defaultExpiresIn = 900;
    return config;
}

}  // namespace

TEST(UploadPresignServiceTest, CreaPresignedPutParaBucketPermitido) {
    UploadPresignService service(testConfig());

    const PresignedUpload upload = service.createPresignedPut(
        "especies-fotos", "canelo.jpg", "image/jpeg", 600);

    EXPECT_EQ(upload.method, "PUT");
    EXPECT_EQ(upload.bucket, "especies-fotos");
    EXPECT_EQ(upload.expiresIn, 600);
    EXPECT_EQ(upload.headers.at("Content-Type"), "image/jpeg");
    EXPECT_EQ(upload.key.rfind("especies/", 0), 0);
    EXPECT_NE(upload.key.find("-canelo.jpg"), std::string::npos);
    EXPECT_EQ(upload.url.rfind("http://localhost:9000/especies-fotos/", 0), 0);
    EXPECT_NE(upload.url.find("X-Amz-Algorithm=AWS4-HMAC-SHA256"),
              std::string::npos);
    EXPECT_NE(upload.url.find("X-Amz-SignedHeaders=content-type%3Bhost"),
              std::string::npos);
    EXPECT_NE(upload.url.find("X-Amz-Signature="), std::string::npos);
}

TEST(UploadPresignServiceTest, UsaPrefijoAvistamientos) {
    UploadPresignService service(testConfig());

    const PresignedUpload upload = service.createPresignedPut(
        "avistamientos-fotos", "foto.webp", "image/webp");

    EXPECT_EQ(upload.key.rfind("avistamientos/", 0), 0);
}

TEST(UploadPresignServiceTest, UsaPrefijoPerfiles) {
    UploadPresignService service(testConfig());

    const PresignedUpload upload = service.createPresignedPut(
        "perfiles-fotos", "avatar.jpg", "image/jpeg");

    EXPECT_EQ(upload.key.rfind("perfiles/", 0), 0);
}

TEST(UploadPresignServiceTest, RechazaBucketNoPermitido) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(service.createPresignedPut("otro-bucket", "foto.jpg",
                                           "image/jpeg"),
                 std::invalid_argument);
}

TEST(UploadPresignServiceTest, RechazaFilenameConRuta) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(service.createPresignedPut("especies-fotos", "../foto.jpg",
                                           "image/jpeg"),
                 std::invalid_argument);
}

TEST(UploadPresignServiceTest, RechazaContentTypeNoImagen) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(service.createPresignedPut("especies-fotos", "foto.txt",
                                           "text/plain"),
                 std::invalid_argument);
}

TEST(UploadPresignServiceTest, RechazaExpiracionFueraDeRango) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(service.createPresignedPut("especies-fotos", "foto.jpg",
                                           "image/jpeg", 10),
                 std::invalid_argument);
    EXPECT_THROW(service.createPresignedPut("especies-fotos", "foto.jpg",
                                           "image/jpeg", 7200),
                 std::invalid_argument);
}

// --- URLs de lectura firmadas (bucket privado de avistamientos) ---

TEST(UploadPresignServiceTest, CreaPresignedGetParaAvistamientos) {
    UploadPresignService service(testConfig());

    const std::string url = service.createPresignedGet(
        "avistamientos-fotos", "avistamientos/2026/08/15/abc-foto.jpg", 600);

    EXPECT_EQ(url.rfind("http://localhost:9000/avistamientos-fotos/"
                        "avistamientos/2026/08/15/abc-foto.jpg?", 0),
              0);
    EXPECT_NE(url.find("X-Amz-Expires=600"), std::string::npos);
    EXPECT_NE(url.find("X-Amz-Signature="), std::string::npos);
    // Sin content-type que firmar: el GET solo firma el host.
    EXPECT_NE(url.find("X-Amz-SignedHeaders=host"), std::string::npos);
}

// La firma se calcula sobre el endpoint público, no sobre el interno: quien
// abre la URL es el teléfono, que no ve la red de Docker.
TEST(UploadPresignServiceTest, PresignedGetUsaElEndpointPublico) {
    UploadStorageConfig config = testConfig();
    config.publicEndpoint = "https://fotos.example.org";
    UploadPresignService service(config);

    const std::string url =
        service.createPresignedGet("avistamientos-fotos", "avistamientos/x.jpg");

    EXPECT_EQ(url.rfind("https://fotos.example.org/avistamientos-fotos/", 0), 0);
}

TEST(UploadPresignServiceTest, PresignedGetRechazaBucketNoPermitido) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(service.createPresignedGet("otro-bucket", "foto.jpg"),
                 std::invalid_argument);
}

// La key viene de la BD, pero pudo escribirla un cliente viejo: firmar una
// travesía de rutas daría acceso a otro bucket.
TEST(UploadPresignServiceTest, PresignedGetRechazaKeyConRuta) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(
        service.createPresignedGet("avistamientos-fotos", "../especies-fotos/x.jpg"),
        std::invalid_argument);
    EXPECT_THROW(service.createPresignedGet("avistamientos-fotos", ""),
                 std::invalid_argument);
}

TEST(UploadPresignServiceTest, PresignedGetRechazaExpiracionFueraDeRango) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(service.createPresignedGet("avistamientos-fotos", "a.jpg", 10),
                 std::invalid_argument);
    EXPECT_THROW(service.createPresignedGet("avistamientos-fotos", "a.jpg", 7200),
                 std::invalid_argument);
}

// --- URLs públicas (buckets de lectura anónima) ---

TEST(UploadPresignServiceTest, CreaUrlPublicaSinFirmaNiCaducidad) {
    UploadPresignService service(testConfig());

    const std::string url = service.createPublicUrl(
        "especies-fotos", "especies/2026/08/16/abc-canelo.jpg");

    EXPECT_EQ(url,
              "http://localhost:9000/especies-fotos/especies/2026/08/16/"
              "abc-canelo.jpg");
    EXPECT_EQ(url.find("X-Amz-Signature="), std::string::npos);
    EXPECT_EQ(url.find('?'), std::string::npos);
}

TEST(UploadPresignServiceTest, UrlPublicaUsaElEndpointPublico) {
    UploadStorageConfig config = testConfig();
    config.publicEndpoint = "https://storage.example.org/";
    UploadPresignService service(config);

    EXPECT_EQ(service.createPublicUrl("perfiles-fotos", "perfiles/a.jpg"),
              "https://storage.example.org/perfiles-fotos/perfiles/a.jpg");
}

// El bucket de encuentros es privado (`mc anonymous set none`): una URL sin
// firma daría 403 y, peor, escondería el error hasta que el usuario ve el hueco.
TEST(UploadPresignServiceTest, UrlPublicaRechazaBucketPrivado) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(
        service.createPublicUrl("avistamientos-fotos", "avistamientos/a.jpg"),
        std::invalid_argument);
}

TEST(UploadPresignServiceTest, UrlPublicaRechazaBucketNoPermitidoYKeyConRuta) {
    UploadPresignService service(testConfig());

    EXPECT_THROW(service.createPublicUrl("otro-bucket", "a.jpg"),
                 std::invalid_argument);
    EXPECT_THROW(service.createPublicUrl("especies-fotos", "../otro/a.jpg"),
                 std::invalid_argument);
    EXPECT_THROW(service.createPublicUrl("especies-fotos", ""),
                 std::invalid_argument);
}

// Un espacio en la key rompería la URL si no se codifica; la barra no, porque
// es separador de ruta real.
TEST(UploadPresignServiceTest, UrlPublicaCodificaLaKeyPeroNoLasBarras) {
    UploadPresignService service(testConfig());

    EXPECT_EQ(service.createPublicUrl("especies-fotos", "especies/mi foto.jpg"),
              "http://localhost:9000/especies-fotos/especies/mi%20foto.jpg");
}
