// Copyright 2025 <Obrero/Obrero>
#include "../../include/controllers/genero_controller.hpp"

#include <pistache/http.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

GeneroController::GeneroController(std::shared_ptr<GeneroService> svc)
    : service(svc) {}

void GeneroController::validarGenero(const Genero& genero) {
  if (!genero.esValida()) {
    throw std::invalid_argument("Los datos del género no son válidos");
  }
}

void GeneroController::getAll(const Pistache::Rest::Request& request,
                              Pistache::Http::ResponseWriter response) {
  try {
    auto generos = service->getAllGeneros();
    json generosArray = json::array();
    for (const auto& genero : generos) {
      generosArray.push_back(genero.toJson());
    }

    json jsonResponse = {{"success", true},
                         {"data", generosArray},
                         {"message", "Géneros obtenidos exitosamente"}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, jsonResponse.dump());

  } catch (const std::exception& e) {
    json errorResponse = {
        {"success", false}, {"data", json::array()}, {"message", e.what()}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error,
                  errorResponse.dump());
  }
}

void GeneroController::getById(const Pistache::Rest::Request& request,
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

    auto genero = service->getGeneroById(id);
    if (!genero) {
      json error = {{"error", "Género no encontrado"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, genero->toJson().dump());
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

void GeneroController::create(const Pistache::Rest::Request& request,
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
    Genero nuevoGenero = Genero::fromJson(requestJson);
    try {
      validarGenero(nuevoGenero);
    } catch (const std::invalid_argument& e) {
      json error = {{"error", e.what()}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto generoCreado = service->createGenero(nuevoGenero);
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Created, generoCreado.toJson().dump());

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

void GeneroController::update(const Pistache::Rest::Request& request,
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
    Genero generoActualizado = Genero::fromJson(requestJson);
    generoActualizado.setId(id);

    try {
      validarGenero(generoActualizado);
    } catch (const std::invalid_argument& e) {
      json error = {{"error", e.what()}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto generoActualizadoResult = service->updateGenero(generoActualizado);
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok,
                  generoActualizadoResult.toJson().dump());

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

void GeneroController::remove(const Pistache::Rest::Request& request,
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

    bool eliminado = service->deleteGenero(id);
    if (!eliminado) {
      json error = {{"error", "Género no encontrado"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    json success = {{"message", "Género eliminado correctamente"}};
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

void GeneroController::uploadImagen(const Pistache::Rest::Request& request,
                                    Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    auto principal_str = request.param(":principal").as<std::string>();
    int genero_id;
    bool es_principal;

    try {
      genero_id = std::stoi(id_str);
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
        service->subirImagen(genero_id, imagen_vector, es_principal);

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

void GeneroController::setImagenPrincipal(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    std::string imagen_url = request.param(":url").as<std::string>();
    int genero_id;

    try {
      genero_id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    bool success = service->setImagenPrincipal(genero_id, imagen_url);
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

void GeneroController::serveImage(const Pistache::Rest::Request& request,
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

    std::string filepath = "./static/images/generos/" + filename;

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

void GeneroController::removeImagen(const Pistache::Rest::Request& request,
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

    std::string filepath = "./static/images/generos/" + filename;

    if (!std::filesystem::exists(filepath)) {
      json error = {{"error", "Imagen no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    // Construir URL de la imagen
    std::string image_url = "/api/images/generos/" + filename;

    // Intentar extraer el ID de género del nombre del archivo
    // Formato esperado: genero_<id>_<timestamp>.<ext>
    int genero_id = -1;
    size_t pos = filename.find("genero_");
    if (pos != std::string::npos) {
      pos += 7;  // longitud de "genero_"
      size_t end_pos = filename.find("_", pos);
      if (end_pos != std::string::npos) {
        try {
          genero_id = std::stoi(filename.substr(pos, end_pos - pos));
        } catch (const std::exception& e) {
          // Si no se puede extraer el ID, continuar con la eliminación del
          // archivo
        }
      }
    }

    // Eliminar del servicio si tenemos el genero_id
    if (genero_id != -1) {
      service->removeImagenFromGenero(genero_id, image_url);
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

void GeneroController::getImagenesByGenero(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    int genero_id;

    try {
      genero_id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    // Verificar que el género existe
    auto genero = service->getGeneroById(genero_id);
    if (!genero) {
      json error = {{"error", "Género no encontrado"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    auto imagenes = service->getImagenesByGeneroId(genero_id);

    json success = {{"message", "Imágenes obtenidas correctamente"},
                    {"genero_id", genero_id},
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
