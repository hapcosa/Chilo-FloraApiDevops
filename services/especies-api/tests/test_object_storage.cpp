#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

#include "services/upload_service.hpp"
#include "utils/object_storage.hpp"

namespace {

ObjectStorageConfig testConfig() {
    return {
        "http://minio:9000",
        "http://localhost:9000",
        "us-east-1",
        "test-access",
        "test-secret",
        "especies-fotos",
        "avistamientos-fotos",
        900,
    };
}

}  // namespace

TEST(ObjectStorageTest, ValidaObjectKeysSeguras) {
    EXPECT_TRUE(ObjectStorageClient::isValidObjectKey(
        "especies/20260610/abc-123_file.jpg"));
    EXPECT_FALSE(ObjectStorageClient::isValidObjectKey(""));
    EXPECT_FALSE(ObjectStorageClient::isValidObjectKey("/especies/a.jpg"));
    EXPECT_FALSE(ObjectStorageClient::isValidObjectKey("especies/../a.jpg"));
    EXPECT_FALSE(ObjectStorageClient::isValidObjectKey("especies//a.jpg"));
    EXPECT_FALSE(ObjectStorageClient::isValidObjectKey("especies/a foto.jpg"));
}

TEST(ObjectStorageTest, GeneraPresignedPutUrlPathStyle) {
    ObjectStorageClient client(testConfig());

    const auto presigned = client.presignPutObject(
        "especies-fotos", "especies/20260610/test.jpg", 600);

    EXPECT_EQ(presigned.bucket, "especies-fotos");
    EXPECT_EQ(presigned.key, "especies/20260610/test.jpg");
    EXPECT_EQ(presigned.expiresIn, 600);
    EXPECT_NE(presigned.url.find(
                  "http://localhost:9000/especies-fotos/especies/20260610/test.jpg"),
              std::string::npos);
    EXPECT_NE(presigned.url.find("X-Amz-Algorithm=AWS4-HMAC-SHA256"),
              std::string::npos);
    EXPECT_NE(presigned.url.find("X-Amz-Signature="), std::string::npos);
}

TEST(ObjectStorageTest, RechazaPresignConKeyInvalida) {
    ObjectStorageClient client(testConfig());

    EXPECT_THROW(client.presignPutObject(
                     "especies-fotos", "especies/../test.jpg", 600),
                 std::invalid_argument);
}

TEST(UploadServiceTest, PresignEspeciesUsaBucketYPrefijoCorrectos) {
    auto client = std::make_shared<ObjectStorageClient>(testConfig());
    UploadService service(client);

    PresignUploadRequest request;
    request.scope = "especies";
    request.contentType = "image/jpeg";
    request.filename = "foto.jpg";
    request.expiresSeconds = 300;

    const auto response = service.createPresignedUpload(request);

    EXPECT_EQ(response.method, "PUT");
    EXPECT_EQ(response.bucket, "especies-fotos");
    EXPECT_TRUE(ObjectStorageClient::hasPrefix(response.key, "especies/"));
    EXPECT_EQ(response.contentType, "image/jpeg");
    EXPECT_EQ(response.expiresIn, 300);
    EXPECT_EQ(response.maxSizeBytes, 10 * 1024 * 1024);
}

TEST(UploadServiceTest, PresignAvistamientosUsaBucketYPrefijoCorrectos) {
    auto client = std::make_shared<ObjectStorageClient>(testConfig());
    UploadService service(client);

    PresignUploadRequest request;
    request.scope = "avistamientos";
    request.contentType = "image/png";

    const auto response = service.createPresignedUpload(request);

    EXPECT_EQ(response.bucket, "avistamientos-fotos");
    EXPECT_TRUE(ObjectStorageClient::hasPrefix(response.key, "avistamientos/"));
}

TEST(UploadServiceTest, RechazaScopeYContentTypeInvalidos) {
    auto client = std::make_shared<ObjectStorageClient>(testConfig());
    UploadService service(client);

    PresignUploadRequest request;
    request.scope = "videos";
    request.contentType = "image/jpeg";
    EXPECT_THROW(service.createPresignedUpload(request), std::invalid_argument);

    request.scope = "especies";
    request.contentType = "application/pdf";
    EXPECT_THROW(service.createPresignedUpload(request), std::invalid_argument);
}
