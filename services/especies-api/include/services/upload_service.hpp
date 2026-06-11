#ifndef UPLOAD_SERVICE_HPP
#define UPLOAD_SERVICE_HPP

#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "../utils/object_storage.hpp"

struct PresignUploadRequest {
    std::string scope;
    std::string contentType;
    std::string filename;
    std::optional<int> expiresSeconds;
};

struct PresignUploadResponse {
    std::string method;
    std::string bucket;
    std::string key;
    std::string url;
    std::string contentType;
    int expiresIn;
    int maxSizeBytes;

    nlohmann::json toJson() const;
};

class UploadService {
public:
    explicit UploadService(std::shared_ptr<ObjectStorageClient> storage);

    PresignUploadResponse createPresignedUpload(
        const PresignUploadRequest& request) const;

    static bool isAllowedContentType(const std::string& contentType);

private:
    std::shared_ptr<ObjectStorageClient> storage;

    std::string bucketForScope(const std::string& scope) const;
    std::string prefixForScope(const std::string& scope) const;
};

#endif // UPLOAD_SERVICE_HPP
