#ifndef UPLOAD_PRESIGN_SERVICE_HPP
#define UPLOAD_PRESIGN_SERVICE_HPP

#include <map>
#include <optional>
#include <string>

struct UploadStorageConfig {
    std::string endpoint;
    std::string publicEndpoint;
    std::string region;
    std::string accessKeyId;
    std::string secretAccessKey;
    std::string especiesBucket;
    std::string avistamientosBucket;
    std::string perfilesBucket;
    int defaultExpiresIn;

    static UploadStorageConfig fromEnvironment();
};

struct PresignedUpload {
    std::string method;
    std::string bucket;
    std::string key;
    std::string url;
    std::map<std::string, std::string> headers;
    int expiresIn;
};

class UploadPresignService {
public:
    explicit UploadPresignService(
        UploadStorageConfig storageConfig =
            UploadStorageConfig::fromEnvironment());

    PresignedUpload createPresignedPut(
        const std::string& bucket,
        const std::string& filename,
        const std::string& contentType,
        std::optional<int> expiresIn = std::nullopt) const;

    bool objectExists(const std::string& bucket, const std::string& key) const;
    const UploadStorageConfig& getConfig() const;

private:
    UploadStorageConfig storageConfig;
};

#endif  // UPLOAD_PRESIGN_SERVICE_HPP
