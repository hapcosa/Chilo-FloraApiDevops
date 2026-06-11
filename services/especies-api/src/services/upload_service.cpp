#include "../../include/services/upload_service.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace {

constexpr int kMaxUploadSizeBytes = 10 * 1024 * 1024;

std::string extensionForContentType(const std::string& contentType) {
    if (contentType == "image/jpeg") return ".jpg";
    if (contentType == "image/png") return ".png";
    if (contentType == "image/webp") return ".webp";
    if (contentType == "image/heif") return ".heif";
    if (contentType == "image/heic") return ".heic";
    throw std::invalid_argument("content_type no permitido");
}

std::string todayUtc() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&time, &tm);

    std::ostringstream out;
    out << std::put_time(&tm, "%Y%m%d");
    return out.str();
}

std::string randomHex() {
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<unsigned long long> dist;

    std::ostringstream out;
    out << std::hex << std::setfill('0')
        << std::setw(16) << dist(rng)
        << std::setw(16) << dist(rng);
    return out.str();
}

}  // namespace

nlohmann::json PresignUploadResponse::toJson() const {
    return {
        {"method", method},
        {"bucket", bucket},
        {"key", key},
        {"url", url},
        {"headers", {{"Content-Type", contentType}}},
        {"expires_in", expiresIn},
        {"max_size_bytes", maxSizeBytes},
    };
}

UploadService::UploadService(std::shared_ptr<ObjectStorageClient> storage)
    : storage(std::move(storage)) {
    if (!this->storage) {
        throw std::invalid_argument("UploadService requiere ObjectStorageClient");
    }
}

PresignUploadResponse UploadService::createPresignedUpload(
    const PresignUploadRequest& request) const {
    const std::string scope = request.scope.empty() ? "especies" : request.scope;
    if (!isAllowedContentType(request.contentType)) {
        throw std::invalid_argument(
            "content_type debe ser image/jpeg, image/png, image/webp, image/heif o image/heic");
    }

    const std::string key = prefixForScope(scope) + "/" + todayUtc() + "/"
        + randomHex() + extensionForContentType(request.contentType);
    const std::string bucket = bucketForScope(scope);
    const auto presigned =
        storage->presignPutObject(bucket, key, request.expiresSeconds);

    return {"PUT",
            presigned.bucket,
            presigned.key,
            presigned.url,
            request.contentType,
            presigned.expiresIn,
            kMaxUploadSizeBytes};
}

bool UploadService::isAllowedContentType(const std::string& contentType) {
    return contentType == "image/jpeg" || contentType == "image/png"
        || contentType == "image/webp" || contentType == "image/heif"
        || contentType == "image/heic";
}

std::string UploadService::bucketForScope(const std::string& scope) const {
    const auto& config = storage->getConfig();
    if (scope == "especies") return config.especiesBucket;
    if (scope == "avistamientos") return config.avistamientosBucket;
    throw std::invalid_argument("scope debe ser 'especies' o 'avistamientos'");
}

std::string UploadService::prefixForScope(const std::string& scope) const {
    if (scope == "especies") return "especies";
    if (scope == "avistamientos") return "avistamientos";
    throw std::invalid_argument("scope debe ser 'especies' o 'avistamientos'");
}
