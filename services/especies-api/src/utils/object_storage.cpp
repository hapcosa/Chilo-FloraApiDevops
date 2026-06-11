#include "../../include/utils/object_storage.hpp"

#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

std::string envOrDefault(const char* key, const std::string& fallback) {
    const char* value = std::getenv(key);
    return value && *value ? value : fallback;
}

int envIntOrDefault(const char* key, int fallback) {
    const char* value = std::getenv(key);
    if (!value || !*value) return fallback;
    try {
        return std::stoi(value);
    } catch (...) {
        return fallback;
    }
}

std::string trimTrailingSlash(std::string value) {
    while (!value.empty() && value.back() == '/') {
        value.pop_back();
    }
    return value;
}

std::string toHex(const unsigned char* data, std::size_t len) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < len; ++i) {
        out << std::setw(2) << static_cast<int>(data[i]);
    }
    return out.str();
}

std::vector<unsigned char> hmacSha256(const std::vector<unsigned char>& key,
                                      const std::string& data) {
    unsigned int len = SHA256_DIGEST_LENGTH;
    std::vector<unsigned char> result(len);
    HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(),
         result.data(), &len);
    result.resize(len);
    return result;
}

std::vector<unsigned char> hmacSha256(const std::string& key,
                                      const std::string& data) {
    std::vector<unsigned char> keyBytes(key.begin(), key.end());
    return hmacSha256(keyBytes, data);
}

std::string sha256Hex(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);
    return toHex(hash, SHA256_DIGEST_LENGTH);
}

std::vector<unsigned char> signingKey(const std::string& secretKey,
                                      const std::string& date,
                                      const std::string& region) {
    auto kDate = hmacSha256("AWS4" + secretKey, date);
    auto kRegion = hmacSha256(kDate, region);
    auto kService = hmacSha256(kRegion, "s3");
    return hmacSha256(kService, "aws4_request");
}

struct Timestamp {
    std::string date;
    std::string amzDate;
};

Timestamp utcTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&time, &tm);

    std::ostringstream date;
    date << std::put_time(&tm, "%Y%m%d");

    std::ostringstream amz;
    amz << std::put_time(&tm, "%Y%m%dT%H%M%SZ");

    return {date.str(), amz.str()};
}

bool isUnreserved(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
        || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.'
        || c == '~';
}

std::string uriEncode(const std::string& value, bool encodeSlash) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0');
    for (unsigned char c : value) {
        if (isUnreserved(c) || (!encodeSlash && c == '/')) {
            out << c;
        } else {
            out << '%' << std::setw(2) << static_cast<int>(c);
        }
    }
    return out.str();
}

std::string hostFromEndpoint(const std::string& endpoint) {
    const auto schemePos = endpoint.find("://");
    const auto start = schemePos == std::string::npos ? 0 : schemePos + 3;
    const auto slash = endpoint.find('/', start);
    return endpoint.substr(start, slash == std::string::npos ? std::string::npos
                                                            : slash - start);
}

std::string canonicalUri(const std::string& bucket, const std::string& key) {
    return "/" + uriEncode(bucket, true) + "/" + uriEncode(key, false);
}

std::string credentialScope(const std::string& date,
                            const std::string& region) {
    return date + "/" + region + "/s3/aws4_request";
}

std::string signatureFor(const std::string& secretKey,
                         const std::string& date,
                         const std::string& region,
                         const std::string& stringToSign) {
    const auto key = signingKey(secretKey, date, region);
    const auto digest = hmacSha256(key, stringToSign);
    return toHex(digest.data(), digest.size());
}

void ensureStorageConfig(const ObjectStorageConfig& config) {
    if (config.endpointInternal.empty() || config.endpointPublic.empty()) {
        throw std::invalid_argument("Object storage endpoint no configurado");
    }
    if (config.accessKey.empty() || config.secretKey.empty()) {
        throw std::invalid_argument("Object storage credentials no configuradas");
    }
    if (config.region.empty()) {
        throw std::invalid_argument("Object storage region no configurada");
    }
}

}  // namespace

ObjectStorageConfig ObjectStorageConfig::fromEnvironment() {
    ObjectStorageConfig config;
    config.endpointInternal = trimTrailingSlash(envOrDefault(
        "OBJECT_STORAGE_ENDPOINT_INTERNAL", "http://minio:9000"));
    config.endpointPublic = trimTrailingSlash(envOrDefault(
        "OBJECT_STORAGE_ENDPOINT_PUBLIC", "http://localhost:9000"));
    config.region = envOrDefault("OBJECT_STORAGE_REGION", "us-east-1");
    config.accessKey = envOrDefault("OBJECT_STORAGE_ACCESS_KEY", "minioadmin");
    config.secretKey = envOrDefault("OBJECT_STORAGE_SECRET_KEY", "minioadmin123");
    config.especiesBucket = envOrDefault("OBJECT_STORAGE_ESPECIES_BUCKET",
                                         "especies-fotos");
    config.avistamientosBucket = envOrDefault(
        "OBJECT_STORAGE_AVISTAMIENTOS_BUCKET", "avistamientos-fotos");
    config.presignExpiresSeconds = envIntOrDefault(
        "OBJECT_STORAGE_PRESIGN_EXPIRES_SECONDS", 900);
    return config;
}

ObjectStorageClient::ObjectStorageClient(ObjectStorageConfig config)
    : config(std::move(config)) {
    ensureStorageConfig(this->config);
}

PresignedUrl ObjectStorageClient::presignPutObject(
    const std::string& bucket,
    const std::string& key,
    std::optional<int> expiresSeconds) const {
    if (!isValidObjectKey(key)) {
        throw std::invalid_argument("Object key inválida");
    }

    const int expires = expiresSeconds.value_or(config.presignExpiresSeconds);
    if (expires < 1 || expires > 604800) {
        throw std::invalid_argument("expiresSeconds debe estar en [1, 604800]");
    }

    const auto ts = utcTimestamp();
    const auto scope = credentialScope(ts.date, config.region);
    const auto encodedCredential =
        uriEncode(config.accessKey + "/" + scope, true);
    const auto host = hostFromEndpoint(config.endpointPublic);
    const auto uri = canonicalUri(bucket, key);

    std::string canonicalQuery =
        "X-Amz-Algorithm=AWS4-HMAC-SHA256"
        "&X-Amz-Credential=" + encodedCredential
        + "&X-Amz-Date=" + ts.amzDate
        + "&X-Amz-Expires=" + std::to_string(expires)
        + "&X-Amz-SignedHeaders=host";

    const std::string canonicalHeaders = "host:" + host + "\n";
    const std::string canonicalRequest =
        "PUT\n" + uri + "\n" + canonicalQuery + "\n"
        + canonicalHeaders + "\n" + "host\n" + "UNSIGNED-PAYLOAD";

    const std::string stringToSign =
        "AWS4-HMAC-SHA256\n" + ts.amzDate + "\n" + scope + "\n"
        + sha256Hex(canonicalRequest);

    const std::string signature =
        signatureFor(config.secretKey, ts.date, config.region, stringToSign);

    return {config.endpointPublic + uri + "?" + canonicalQuery
                + "&X-Amz-Signature=" + signature,
            key,
            bucket,
            expires};
}

bool ObjectStorageClient::objectExists(const std::string& bucket,
                                       const std::string& key) const {
    if (!isValidObjectKey(key)) {
        throw std::invalid_argument("Object key inválida");
    }

    const auto ts = utcTimestamp();
    const auto scope = credentialScope(ts.date, config.region);
    const auto host = hostFromEndpoint(config.endpointInternal);
    const auto uri = canonicalUri(bucket, key);
    const std::string payloadHash = "UNSIGNED-PAYLOAD";
    const std::string signedHeaders = "host;x-amz-content-sha256;x-amz-date";

    const std::string canonicalHeaders =
        "host:" + host + "\n"
        + "x-amz-content-sha256:" + payloadHash + "\n"
        + "x-amz-date:" + ts.amzDate + "\n";

    const std::string canonicalRequest =
        "HEAD\n" + uri + "\n\n" + canonicalHeaders + "\n"
        + signedHeaders + "\n" + payloadHash;

    const std::string stringToSign =
        "AWS4-HMAC-SHA256\n" + ts.amzDate + "\n" + scope + "\n"
        + sha256Hex(canonicalRequest);

    const std::string signature =
        signatureFor(config.secretKey, ts.date, config.region, stringToSign);

    const std::string authorization =
        "Authorization: AWS4-HMAC-SHA256 Credential=" + config.accessKey + "/"
        + scope + ", SignedHeaders=" + signedHeaders + ", Signature="
        + signature;
    const std::string amzDate = "x-amz-date: " + ts.amzDate;
    const std::string amzPayload = "x-amz-content-sha256: " + payloadHash;

    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("No se pudo inicializar libcurl");
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, authorization.c_str());
    headers = curl_slist_append(headers, amzDate.c_str());
    headers = curl_slist_append(headers, amzPayload.c_str());

    long status = 0;
    const std::string url = config.endpointInternal + uri;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    const CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("No se pudo verificar objeto en storage: "
                                 + std::string(curl_easy_strerror(res)));
    }

    if (status == 200) return true;
    if (status == 404) return false;

    throw std::runtime_error("Storage respondió estado inesperado en HEAD: "
                             + std::to_string(status));
}

bool ObjectStorageClient::isValidObjectKey(const std::string& key) {
    if (key.empty() || key.size() > 500) return false;
    if (key.front() == '/' || key.back() == '/') return false;
    if (key.find("..") != std::string::npos) return false;
    if (key.find("//") != std::string::npos) return false;

    return std::all_of(key.begin(), key.end(), [](unsigned char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.'
            || c == '/';
    });
}

bool ObjectStorageClient::hasPrefix(const std::string& value,
                                    const std::string& prefix) {
    return value.size() >= prefix.size()
        && value.compare(0, prefix.size(), prefix) == 0;
}
