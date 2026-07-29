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
