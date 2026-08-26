// Copyright 2025 <Obrero/Obrero>
#include "../../include/controllers/familia_controller.hpp"
#include "../../include/utils/query_params.hpp"

#include <pistache/http.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
using json = nlohmann::json;

FamiliaController::FamiliaController(std::shared_ptr<FamiliaService> svc)
    : service(svc) {}

void FamiliaController::validarFamilia(const Familia& familia) {
  if (!familia.esValida()) {
    throw std::invalid_argument("Los datos de la familia no son válidos");
  }
}

void FamiliaController::getAll(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  try {
    auto query = request.query();
    std::vector<Familia> familias;
    if (query.has("reino")) {
      const std::string reinoStr = utils::percentDecode(query.get("reino").value());
      familias = service->getFamiliasByReino(reinoFromString(reinoStr));
    } else {
      familias = service->getAllFamilias();
    }

    json familiasArray = json::array();
    for (const auto& familia : familias) {
      familiasArray.push_back(familia.toJson());
    }

    json jsonResponse = {{"success", true},
                         {"data", familiasArray},
                         {"message", "Familias obtenidas exitosamente"}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, jsonResponse.dump());

  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json errorResponse = {
        {"success", false}, {"data", json::array()}, {"message", e.what()}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error,
                  errorResponse.dump());
  }
}

void FamiliaController::getById(const Pistache::Rest::Request& request,
                                Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    int id;

    try {
      id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto familia = service->findFamiliaById(id);
    if (!familia) {
      json error = {{"error", "Familia no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, familia->toJson().dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void FamiliaController::create(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  try {
    auto body = request.body();
    if (body.empty()) {
      json error = {{"error", "El cuerpo de la petición no puede estar vacío"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto requestJson = json::parse(body);
    Familia nuevaFamilia = Familia::fromJson(requestJson);
    try {
      validarFamilia(nuevaFamilia);
    } catch (const std::invalid_argument& e) {
      json error = {{"error", e.what()}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto familiaCreada = service->createFamilia(nuevaFamilia);
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Created, familiaCreada.toJson().dump());

  } catch (const json::parse_error& e) {
    json error = {{"error", "JSON inválido: " + std::string(e.what())}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Conflict, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void FamiliaController::update(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    int id;

    try {
      id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto body = request.body();
    if (body.empty()) {
      json error = {{"error", "El cuerpo de la petición no puede estar vacío"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto requestJson = json::parse(body);
    Familia familiaActualizada = Familia::fromJson(requestJson);
    familiaActualizada.setId(id);

    try {
      validarFamilia(familiaActualizada);
    } catch (const std::invalid_argument& e) {
      json error = {{"error", e.what()}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto familiaActualizadaResult = service->updateFamilia(familiaActualizada);
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok,
                  familiaActualizadaResult.toJson().dump());

  } catch (const json::parse_error& e) {
    json error = {{"error", "JSON inválido: " + std::string(e.what())}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void FamiliaController::remove(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    int id;

    try {
      id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    bool eliminado = service->deleteFamilia(id);
    if (!eliminado) {
      json error = {{"error", "Familia no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    json success = {{"message", "Familia eliminada correctamente"}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, success.dump());

  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void FamiliaController::searchByNombre(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto query = request.query();
    if (!query.has("nombre") || !query.has("reino")) {
      json error = {
          {"error",
           "Los parámetros 'reino' y 'nombre' son obligatorios (clave única "
           "tras la migración multi-reino)"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    std::string nombre = utils::percentDecode(query.get("nombre").value());
    std::string reinoStr = utils::percentDecode(query.get("reino").value());
    if (nombre.empty() || reinoStr.empty()) {
      json error = {
          {"error", "'reino' y 'nombre' no pueden estar vacíos"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto familia = service->findByNombre(reinoFromString(reinoStr), nombre);

    if (!familia) {
      json error = {{"error", "Familia no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, familia->toJson().dump());

  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void FamiliaController::uploadImagen(const Pistache::Rest::Request& request,
                                     Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    auto principal_str = request.param(":principal").as<std::string>();
    int familia_id;
    bool es_principal;

    try {
      familia_id = std::stoi(id_str);
      es_principal = (principal_str == "true" || principal_str == "1");
    } catch (const std::exception& e) {
      json error = {
          {"error",
           "ID debe ser un número válido y principal debe ser true/false"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto imagen_data = request.body();
    const size_t MAX_SIZE = 10 * 1024 * 1024;  // 10MB
    if (imagen_data.size() > MAX_SIZE) {
      json error = {
          {"error", "La imagen es demasiado grande. Máximo 10MB permitido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }
    if (imagen_data.empty()) {
      json error = {{"error", "Los datos de la imagen no pueden estar vacíos"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    // Convertir string a vector<uint8_t>
    std::vector<uint8_t> imagen_vector(imagen_data.begin(), imagen_data.end());

    std::string imagen_url =
        service->subirImagen(familia_id, imagen_vector, es_principal);

    json success = {{"message", "Imagen subida correctamente"},
                    {"url", imagen_url}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, success.dump());

  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void FamiliaController::setImagenPrincipal(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    std::string imagen_url = request.param(":url").as<std::string>();
    int familia_id;

    try {
      familia_id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    bool success = service->setImagenPrincipal(familia_id, imagen_url);
    if (!success) {
      json error = {
          {"error", "No se pudo establecer la imagen como principal"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    json successResponse = {
        {"message", "Imagen principal cambiada correctamente"},
        {"url", imagen_url}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, successResponse.dump());

  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void FamiliaController::serveImage(const Pistache::Rest::Request& request,
                                   Pistache::Http::ResponseWriter response) {
  try {
    std::string filename = request.param(":filename").as<std::string>();

    // Validar nombre de archivo para evitar path traversal attacks
    if (filename.find("..") != std::string::npos ||
        filename.find("/") != std::string::npos ||
        filename.find("\\") != std::string::npos) {
      json error = {{"error", "Nombre de archivo inválido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    std::string filepath = "./static/images/familias/" + filename;

    if (!std::filesystem::exists(filepath)) {
      json error = {{"error", "Imagen no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    // Leer archivo
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
      json error = {{"error", "Error al leer imagen"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
      return;
    }

    // Obtener contenido del archivo
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Determinar tipo de contenido basado en la extensión
    std::string contentType = "image/jpeg";  // Por defecto JPEG
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   ::tolower);

    if (extension == "png") {
      contentType = "image/png";
    } else if (extension == "gif") {
      contentType = "image/gif";
    } else if (extension == "webp") {
      contentType = "image/webp";
    }

    // Establecer headers apropiados para la imagen
    response.headers().addRaw(
        Pistache::Http::Header::Raw("Content-Type", contentType));
    response.headers().addRaw(
        Pistache::Http::Header::Raw("Cache-Control", "max-age=3600"));
    response.headers().addRaw(Pistache::Http::Header::Raw(
        "Content-Length", std::to_string(content.size())));

    response.send(Pistache::Http::Code::Ok, content);

  } catch (const std::exception& e) {
    json error = {{"error", "Error interno del servidor"}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

// IMPLEMENTACIÓN COMPLETA DEL MÉTODO removeImagen
void FamiliaController::removeImagen(const Pistache::Rest::Request& request,
                                     Pistache::Http::ResponseWriter response) {
  try {
    std::string filename = request.param(":filename").as<std::string>();

    // Validar nombre de archivo para evitar path traversal attacks
    if (filename.find("..") != std::string::npos ||
        filename.find("/") != std::string::npos ||
        filename.find("\\") != std::string::npos) {
      json error = {{"error", "Nombre de archivo inválido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    // Extraer familia_id del filename si está en el formato esperado
    // Asumiendo que el filename contiene información de la familia
    std::string filepath = "./static/images/familias/" + filename;

    if (!std::filesystem::exists(filepath)) {
      json error = {{"error", "Imagen no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    // Construir URL de la imagen
    std::string image_url = "/api/images/familias/" + filename;

    // Intentar extraer el ID de familia del nombre del archivo
    // Formato esperado: familia_<id>_<timestamp>.<ext>
    int familia_id = -1;
    size_t pos = filename.find("familia_");
    if (pos != std::string::npos) {
      pos += 8;  // longitud de "familia_"
      size_t end_pos = filename.find("_", pos);
      if (end_pos != std::string::npos) {
        try {
          familia_id = std::stoi(filename.substr(pos, end_pos - pos));
        } catch (const std::exception& e) {
          // Si no se puede extraer el ID, continuar con la eliminación del
          // archivo
        }
      }
    }

    // Eliminar del servicio si tenemos el familia_id
    if (familia_id != -1) {
      service->removeImagenFromFamilia(familia_id, image_url);
    }

    // Eliminar archivo físico
    try {
      std::filesystem::remove(filepath);
    } catch (const std::filesystem::filesystem_error& e) {
      json error = {
          {"error", "Error al eliminar el archivo: " + std::string(e.what())}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
      return;
    }

    json success = {{"message", "Imagen eliminada correctamente"},
                    {"filename", filename}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, success.dump());

  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", "Error interno del servidor"}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

// NUEVO MÉTODO: Obtener todas las imágenes de una familia
void FamiliaController::getImagenesByFamilia(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    int familia_id;

    try {
      familia_id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    // Verificar que la familia existe
    auto familia = service->findFamiliaById(familia_id);
    if (!familia) {
      json error = {{"error", "Familia no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    auto imagenes = service->getImagenesByFamiliaId(familia_id);

    json success = {{"message", "Imágenes obtenidas correctamente"},
                    {"familia_id", familia_id},
                    {"imagenes", imagenes}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, success.dump());

  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", "Error interno del servidor"}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}
