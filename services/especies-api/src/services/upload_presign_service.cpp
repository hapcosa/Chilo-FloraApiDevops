#include "../../include/services/upload_presign_service.hpp"

#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

std::string envOr(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    if (value == nullptr || std::string(value).empty()) {
        return fallback;
    }
    return value;
}

int envIntOr(const char* name, int fallback) {
    const char* value = std::getenv(name);
    if (value == nullptr) {
        return fallback;
    }
    try {
        return std::stoi(value);
    } catch (...) {
        return fallback;
    }
}

std::string hexEncode(const unsigned char* data, std::size_t length) {
    std::ostringstream output;
    output << std::hex << std::setfill('0');
    for (std::size_t index = 0; index < length; ++index) {
        output << std::setw(2) << static_cast<int>(data[index]);
    }
    return output.str();
}

std::string hexEncode(const std::vector<unsigned char>& data) {
    return hexEncode(data.data(), data.size());
}

std::vector<unsigned char> hmacSha256(
    const std::vector<unsigned char>& keyBytes,
    const std::string& message) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digestLength = 0;

    HMAC(EVP_sha256(), keyBytes.data(), static_cast<int>(keyBytes.size()),
         reinterpret_cast<const unsigned char*>(message.data()),
         message.size(), digest, &digestLength);

    return std::vector<unsigned char>(digest, digest + digestLength);
}

std::vector<unsigned char> hmacSha256(
    const std::string& key,
    const std::string& message) {
    return hmacSha256(std::vector<unsigned char>(key.begin(), key.end()),
                      message);
}

std::string sha256Hex(const std::string& message) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(message.data()),
           message.size(), digest);
    return hexEncode(digest, SHA256_DIGEST_LENGTH);
}

bool isUnreserved(unsigned char character) {
    return std::isalnum(character) || character == '-' || character == '_' ||
           character == '.' || character == '~';
}

std::string uriEncode(const std::string& value, bool encodeSlash) {
    std::ostringstream output;
    output << std::uppercase << std::hex << std::setfill('0');

    for (unsigned char character : value) {
        if (isUnreserved(character) ||
            (!encodeSlash && character == '/')) {
            output << character;
        } else {
            output << '%' << std::setw(2)
                   << static_cast<int>(character);
        }
    }

    return output.str();
}

std::string normalizeEndpoint(std::string endpoint) {
    while (!endpoint.empty() && endpoint.back() == '/') {
        endpoint.pop_back();
    }
    if (endpoint.empty()) {
        throw std::runtime_error("S3_PUBLIC_ENDPOINT no está configurado");
    }
    return endpoint;
}

std::string hostFromEndpoint(const std::string& endpoint) {
    std::string host = endpoint;
    const std::string httpPrefix = "http://";
    const std::string httpsPrefix = "https://";

    if (host.rfind(httpPrefix, 0) == 0) {
        host = host.substr(httpPrefix.size());
    } else if (host.rfind(httpsPrefix, 0) == 0) {
        host = host.substr(httpsPrefix.size());
    }

    const auto slashPosition = host.find('/');
    if (slashPosition != std::string::npos) {
        host = host.substr(0, slashPosition);
    }

    if (host.empty()) {
        throw std::runtime_error("S3_PUBLIC_ENDPOINT no tiene host válido");
    }

    return host;
}

std::string sanitizeFilename(const std::string& filename) {
    if (filename.empty() || filename.size() > 180) {
        throw std::invalid_argument("'filename' debe tener entre 1 y 180 caracteres");
    }
    if (filename.find('/') != std::string::npos ||
        filename.find('\\') != std::string::npos ||
        filename.find("..") != std::string::npos) {
        throw std::invalid_argument("'filename' no puede contener rutas");
    }

    std::string sanitized;
    sanitized.reserve(filename.size());
    for (unsigned char character : filename) {
        if (std::isalnum(character) || character == '.' ||
            character == '-' || character == '_') {
            sanitized.push_back(static_cast<char>(character));
        } else {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty() || sanitized == "." || sanitized == "..") {
        throw std::invalid_argument("'filename' no es válido");
    }

    return sanitized;
}

bool isAllowedContentType(const std::string& contentType) {
    static const std::vector<std::string> allowedTypes = {
        "image/jpeg", "image/png", "image/webp", "image/heic", "image/heif"};

    return std::find(allowedTypes.begin(), allowedTypes.end(), contentType) !=
           allowedTypes.end();
}

std::string randomHex(std::size_t bytesLength) {
    std::vector<unsigned char> randomBytes(bytesLength);
    if (RAND_bytes(randomBytes.data(), static_cast<int>(randomBytes.size())) !=
        1) {
        throw std::runtime_error("No se pudo generar entropía para la key");
    }
    return hexEncode(randomBytes);
}

std::tm utcNow() {
    std::time_t currentTime = std::time(nullptr);
    std::tm utcTime {};
    gmtime_r(&currentTime, &utcTime);
    return utcTime;
}

std::string formatTime(const std::tm& utcTime, const char* format) {
    std::ostringstream output;
    output << std::put_time(&utcTime, format);
    return output.str();
}

std::string queryString(const std::map<std::string, std::string>& params) {
    std::ostringstream output;
    bool first = true;
    for (const auto& param : params) {
        if (!first) {
            output << '&';
        }
        first = false;
        output << uriEncode(param.first, true) << '='
               << uriEncode(param.second, true);
    }
    return output.str();
}

std::string joinedHeaderNames(
    const std::map<std::string, std::string>& headers) {
    std::ostringstream output;
    bool first = true;
    for (const auto& header : headers) {
        if (!first) {
            output << ';';
        }
        first = false;
        output << header.first;
    }
    return output.str();
}

std::string canonicalHeaders(
    const std::map<std::string, std::string>& headers) {
    std::ostringstream output;
    for (const auto& header : headers) {
        output << header.first << ':' << header.second << '\n';
    }
    return output.str();
}

std::vector<unsigned char> signingKey(const std::string& secretAccessKey,
                                      const std::string& dateStamp,
                                      const std::string& region) {
    const auto dateKey = hmacSha256("AWS4" + secretAccessKey, dateStamp);
    const auto regionKey = hmacSha256(dateKey, region);
    const auto serviceKey = hmacSha256(regionKey, "s3");
    return hmacSha256(serviceKey, "aws4_request");
}

struct SignedUrl {
    std::string url;
};

SignedUrl createSignedUrl(const UploadStorageConfig& storageConfig,
                          const std::string& method,
                          const std::string& bucket,
                          const std::string& key,
                          const std::string& endpoint,
                          std::map<std::string, std::string> headers,
                          int expiresIn) {
    const std::tm now = utcNow();
    const std::string amzDate = formatTime(now, "%Y%m%dT%H%M%SZ");
    const std::string dateStamp = formatTime(now, "%Y%m%d");
    const std::string normalizedEndpoint = normalizeEndpoint(endpoint);
    const std::string host = hostFromEndpoint(normalizedEndpoint);
    const std::string credentialScope =
        dateStamp + "/" + storageConfig.region + "/s3/aws4_request";

    headers["host"] = host;
    const std::string signedHeaders = joinedHeaderNames(headers);

    std::map<std::string, std::string> params = {
        {"X-Amz-Algorithm", "AWS4-HMAC-SHA256"},
        {"X-Amz-Credential", storageConfig.accessKeyId + "/" + credentialScope},
        {"X-Amz-Date", amzDate},
        {"X-Amz-Expires", std::to_string(expiresIn)},
        {"X-Amz-SignedHeaders", signedHeaders},
    };

    const std::string canonicalUri =
        "/" + uriEncode(bucket + "/" + key, false);
    const std::string canonicalQueryString = queryString(params);
    const std::string canonicalRequest =
        method + "\n" + canonicalUri + "\n" + canonicalQueryString + "\n" +
        canonicalHeaders(headers) + "\n" + signedHeaders +
        "\nUNSIGNED-PAYLOAD";
    const std::string stringToSign =
        "AWS4-HMAC-SHA256\n" + amzDate + "\n" + credentialScope + "\n" +
        sha256Hex(canonicalRequest);
    const auto keyBytes =
        signingKey(storageConfig.secretAccessKey, dateStamp,
                   storageConfig.region);
    const std::string signature = hexEncode(hmacSha256(keyBytes, stringToSign));

    return {normalizedEndpoint + canonicalUri + "?" + canonicalQueryString +
            "&X-Amz-Signature=" + signature};
}

void validateAllowedBucket(const UploadStorageConfig& storageConfig,
                           const std::string& bucket) {
    if (bucket != storageConfig.especiesBucket &&
        bucket != storageConfig.avistamientosBucket &&
        bucket != storageConfig.perfilesBucket) {
        throw std::invalid_argument("'bucket' no está permitido");
    }
}

void validateObjectKeyFormat(const std::string& key) {
    if (key.empty() || key.size() > 500) {
        throw std::invalid_argument("'key' debe tener entre 1 y 500 caracteres");
    }
    if (key.front() == '/' || key.find('\\') != std::string::npos ||
        key.find("..") != std::string::npos) {
        throw std::invalid_argument("'key' no puede contener rutas inválidas");
    }
    for (unsigned char character : key) {
        if (std::iscntrl(character)) {
            throw std::invalid_argument("'key' contiene caracteres inválidos");
        }
    }
}

}  // namespace

UploadStorageConfig UploadStorageConfig::fromEnvironment() {
    UploadStorageConfig config;
    config.endpoint = envOr("S3_ENDPOINT", "http://localhost:9000");
    config.publicEndpoint = envOr("S3_PUBLIC_ENDPOINT", config.endpoint);
    config.region = envOr("S3_REGION", "us-east-1");
    config.accessKeyId = envOr("S3_ACCESS_KEY_ID", "minioadmin");
    config.secretAccessKey = envOr("S3_SECRET_ACCESS_KEY", "minioadmin123");
    config.especiesBucket =
        envOr("S3_BUCKET_ESPECIES", "especies-fotos");
    config.avistamientosBucket =
        envOr("S3_BUCKET_AVISTAMIENTOS", "avistamientos-fotos");
    config.perfilesBucket = envOr("S3_BUCKET_PERFILES", "perfiles-fotos");
    config.defaultExpiresIn = envIntOr("S3_PRESIGN_EXPIRES_SECONDS", 900);
    return config;
}

UploadPresignService::UploadPresignService(
    UploadStorageConfig storageConfig)
    : storageConfig(std::move(storageConfig)) {}

PresignedUpload UploadPresignService::createPresignedPut(
    const std::string& bucket,
    const std::string& filename,
    const std::string& contentType,
    std::optional<int> expiresIn) const {
    validateAllowedBucket(storageConfig, bucket);
    if (!isAllowedContentType(contentType)) {
        throw std::invalid_argument("'content_type' debe ser una imagen soportada");
    }
    if (storageConfig.accessKeyId.empty() ||
        storageConfig.secretAccessKey.empty()) {
        throw std::runtime_error("Credenciales S3 no configuradas");
    }

    const int effectiveExpiresIn =
        expiresIn.value_or(storageConfig.defaultExpiresIn);
    if (effectiveExpiresIn < 60 || effectiveExpiresIn > 3600) {
        throw std::invalid_argument("'expires_in' debe estar entre 60 y 3600 segundos");
    }

    const std::tm now = utcNow();
    const std::string datePath = formatTime(now, "%Y/%m/%d");
    std::string prefix = "avistamientos";
    if (bucket == storageConfig.especiesBucket) {
        prefix = "especies";
    } else if (bucket == storageConfig.perfilesBucket) {
        prefix = "perfiles";
    }
    const std::string key = prefix + "/" + datePath + "/" + randomHex(16) +
                            "-" + sanitizeFilename(filename);

    const SignedUrl signedUrl =
        createSignedUrl(storageConfig, "PUT", bucket, key,
                        storageConfig.publicEndpoint,
                        {{"content-type", contentType}}, effectiveExpiresIn);

    PresignedUpload upload;
    upload.method = "PUT";
    upload.bucket = bucket;
    upload.key = key;
    upload.url = signedUrl.url;
    upload.headers = {{"Content-Type", contentType}};
    upload.expiresIn = effectiveExpiresIn;
    return upload;
}

bool UploadPresignService::objectExists(const std::string& bucket,
                                        const std::string& key) const {
    validateAllowedBucket(storageConfig, bucket);
    validateObjectKeyFormat(key);
    if (storageConfig.accessKeyId.empty() ||
        storageConfig.secretAccessKey.empty()) {
        throw std::runtime_error("Credenciales S3 no configuradas");
    }

    const SignedUrl signedUrl =
        createSignedUrl(storageConfig, "HEAD", bucket, key,
                        storageConfig.endpoint, {}, 60);

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        throw std::runtime_error("No se pudo inicializar libcurl");
    }

    curl_easy_setopt(curl, CURLOPT_URL, signedUrl.url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);

    const CURLcode result = curl_easy_perform(curl);
    long responseCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK) {
        throw std::runtime_error("No se pudo consultar object storage");
    }
    if (responseCode == 200) {
        return true;
    }
    if (responseCode == 404) {
        return false;
    }

    throw std::runtime_error(
        "Object storage respondió HTTP " + std::to_string(responseCode));
}

const UploadStorageConfig& UploadPresignService::getConfig() const {
    return storageConfig;
}
