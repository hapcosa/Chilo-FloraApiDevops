#ifndef OBJECT_STORAGE_HPP
#define OBJECT_STORAGE_HPP

#include <optional>
#include <string>

struct ObjectStorageConfig {
    std::string endpointInternal;
    std::string endpointPublic;
    std::string region;
    std::string accessKey;
    std::string secretKey;
    std::string especiesBucket;
    std::string avistamientosBucket;
    int presignExpiresSeconds;

    static ObjectStorageConfig fromEnvironment();
};

struct PresignedUrl {
    std::string url;
    std::string key;
    std::string bucket;
    int expiresIn;
};

class ObjectStorageClient {
public:
    explicit ObjectStorageClient(ObjectStorageConfig config);

    const ObjectStorageConfig& getConfig() const { return config; }

    PresignedUrl presignPutObject(const std::string& bucket,
                                  const std::string& key,
                                  std::optional<int> expiresSeconds = std::nullopt) const;

    bool objectExists(const std::string& bucket, const std::string& key) const;

    static bool isValidObjectKey(const std::string& key);
    static bool hasPrefix(const std::string& value, const std::string& prefix);

private:
    ObjectStorageConfig config;
};

#endif // OBJECT_STORAGE_HPP
