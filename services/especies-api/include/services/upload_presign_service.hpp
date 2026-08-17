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

    // URL de lectura de corta duración. `avistamientos-fotos` es un bucket
    // privado (`mc anonymous set none`), así que sin firma no hay forma de
    // mostrar la foto de un encuentro ajeno. A diferencia del PUT, aquí la key
    // ya existe: la elige el servidor al firmar la subida, no el cliente.
    std::string createPresignedGet(
        const std::string& bucket,
        const std::string& key,
        std::optional<int> expiresIn = std::nullopt) const;

    // URL de lectura sin firma ni caducidad. Solo vale para los buckets
    // públicos (`mc anonymous set download`): `especies-fotos` y
    // `perfiles-fotos`. Firmarlos sería peor —una URL distinta en cada
    // respuesta no la cachea ni el CDN ni el cliente, y el catálogo devuelve
    // decenas de fotos por página—. Para `avistamientos-fotos`, que es privado,
    // sigue haciendo falta createPresignedGet.
    std::string createPublicUrl(const std::string& bucket,
                                const std::string& key) const;

    bool objectExists(const std::string& bucket, const std::string& key) const;
    const UploadStorageConfig& getConfig() const;

private:
    UploadStorageConfig storageConfig;
};

#endif  // UPLOAD_PRESIGN_SERVICE_HPP
