// Copyright 2025 <Obrero/Obrero>
#include "../../include/controllers/especie_controller.hpp"
#include "../../include/utils/query_params.hpp"

#include <pistache/http.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
using json = nlohmann::json;

namespace {

// Parsea un query param entero opcional. Lanza std::invalid_argument si
// el valor no es numérico. Devuelve std::nullopt si el param no viene.
std::optional<int> queryInt(const Pistache::Http::Uri::Query& q,
                             const std::string& key) {
    if (!q.has(key)) return std::nullopt;
    const auto raw = q.get(key).value();
    try {
        return std::stoi(raw);
    } catch (...) {
        throw std::invalid_argument("Parámetro '" + key + "' debe ser un entero");
    }
}

std::optional<bool> queryBool(const Pistache::Http::Uri::Query& q,
                               const std::string& key) {
    if (!q.has(key)) return std::nullopt;
    const auto raw = q.get(key).value();
    if (raw == "true" || raw == "1")  return true;
    if (raw == "false" || raw == "0") return false;
    throw std::invalid_argument(
        "Parámetro '" + key + "' debe ser 'true' o 'false'");
}

// Decodifica el valor: Pistache lo entrega tal como viajó por la red.
std::optional<std::string> queryStr(const Pistache::Http::Uri::Query& q,
                                     const std::string& key) {
    if (!q.has(key)) return std::nullopt;
    return utils::percentDecode(q.get(key).value());
}

}  // namespace

EspecieController::EspecieController(std::shared_ptr<EspecieService> svc,
                                     std::shared_ptr<ModeracionService> moderacion)
    : service(svc), moderacion(moderacion) {}

std::optional<RequestIdentity> EspecieController::requireSesion(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter& response) {
  auto identity = extractIdentity(request);
  if (!identity) {
    json error = {{"error", "No se pudo verificar la sesión del usuario"}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Unauthorized, error.dump());
    return std::nullopt;
  }
  return identity;
}

bool EspecieController::requireCuradorDeCategoria(
    const RequestIdentity& identity,
    std::optional<int> categoriaId,
    Pistache::Http::ResponseWriter& response) {
  if (moderacion->puedeEditarCategoria(identity.userId, identity.role,
                                       categoriaId)) {
    return true;
  }

  // Sin categoría no hay nada sobre lo que demostrar curaduría; el mensaje
  // distingue los dos casos porque la acción del cliente es distinta.
  const std::string mensaje =
      categoriaId ? "No tienes permiso de curaduría sobre esta categoría"
                  : "Debes indicar una 'categoria_id' sobre la que tengas curaduría";

  json error = {{"error", mensaje}};
  response.headers().add<Pistache::Http::Header::ContentType>(
      MIME(Application, Json));
  response.send(Pistache::Http::Code::Forbidden, error.dump());
  return false;
}

EspecieVisibilidad EspecieController::visibilidadDe(
    const std::optional<RequestIdentity>& identity) {
  EspecieVisibilidad visibilidad;
  if (!identity) return visibilidad;  // sin sesión: solo lo publicado

  if (identity->canModerate()) {
    visibilidad.verTodo = true;
    return visibilidad;
  }

  // Un curador ve sus propios borradores. Se resuelve contra la BD porque la
  // curaduría no viaja en las cabeceras del gateway (solo id y rol).
  for (const auto& categoria : moderacion->categoriasDe(identity->userId)) {
    visibilidad.categoriasCuradas.push_back(categoria.getId());
  }
  return visibilidad;
}

void EspecieController::validarEspecie(const Especie& especie) {
  if (!especie.esValida()) {
    throw std::invalid_argument("Los datos de la especie no son válidos");
  }
}

void EspecieController::getAll(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  try {
    const auto query = request.query();

    EspecieFilters filters;
    if (auto v = queryStr(query, "reino"))         filters.reino = reinoFromString(*v);
    if (auto v = queryInt(query, "genero_id"))     filters.genero_id = *v;
    if (auto v = queryInt(query, "familia_id"))    filters.familia_id = *v;
    if (auto v = queryInt(query, "categoria_id"))  filters.categoria_id = *v;
    if (auto v = queryStr(query, "conservacion"))  filters.conservacion = *v;
    if (auto v = queryBool(query, "endemica"))     filters.endemica = *v;
    if (auto v = queryStr(query, "q"))             filters.q = *v;
    if (auto v = queryInt(query, "limit"))         filters.limit = *v;
    if (auto v = queryInt(query, "offset"))        filters.offset = *v;
    if (auto v = queryStr(query, "orderby"))       filters.orderby = *v;
    if (auto v = queryStr(query, "orderdir"))      filters.orderdir = *v;
    if (auto v = queryStr(query, "estado"))        filters.estado = especieEstadoFromString(*v);

    // El listado es público (y alimenta el cache del móvil): quien no cura
    // nada solo ve fichas publicadas, pida lo que pida en ?estado=.
    filters.visibilidad = visibilidadDe(extractIdentity(request));

    auto result = service->searchEspecies(filters);

    json especiesArray = json::array();
    for (const auto& especie : result.data) {
      especiesArray.push_back(especie.toJson());
    }

    json jsonResponse = {
        {"success", true},
        {"data", especiesArray},
        {"pagination", {{"limit", filters.limit},
                        {"offset", filters.offset},
                        {"total", result.total}}},
        {"message", "Especies obtenidas exitosamente"}};

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

void EspecieController::getById(const Pistache::Rest::Request& request,
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

    auto especie = service->getEspecieById(id);
    if (!especie) {
      json error = {{"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    // Un borrador ajeno no existe para quien lo pide: 404 y no 403, para no
    // convertir el endpoint en un oráculo de qué fichas se están redactando.
    if (especie->esBorrador()) {
      const auto identity = extractIdentity(request);
      const bool puedeVerlo =
          identity && moderacion->puedeEditarCategoria(
                          identity->userId, identity->role,
                          especie->getCategoriaId());
      if (!puedeVerlo) {
        json error = {{"error", "Especie no encontrada"}};
        response.headers().add<Pistache::Http::Header::ContentType>(
            MIME(Application, Json));
        response.send(Pistache::Http::Code::Not_Found, error.dump());
        return;
      }
    }

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, especie->toJson().dump());
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
// searchByGenero eliminado: usar GET /api/especies?genero_id=N (Fase 1 paso 6).

void EspecieController::create(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  auto identity = requireSesion(request, response);
  if (!identity) return;

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

    // El permiso se decide sobre la categoría que pide el cuerpo, así que hay
    // que leerla antes; pero antes de validar la ficha entera, o quien no está
    // autorizado recibiría un 400 de validación en vez de un 403.
    std::optional<int> categoriaSolicitada;
    if (requestJson.contains("categoria_id") &&
        requestJson["categoria_id"].is_number_integer()) {
      categoriaSolicitada = requestJson["categoria_id"].get<int>();
    }
    if (!requireCuradorDeCategoria(*identity, categoriaSolicitada, response)) {
      return;
    }

    Especie nuevaEspecie = Especie::fromJson(requestJson);

    // creado_por/revisado_por vienen de la identidad verificada, nunca del
    // cliente: nadie puede autoatribuirse como otro usuario ni auto-revisarse.
    nuevaEspecie.setCreadoPor(identity->userId);
    nuevaEspecie.setRevisadoPor(std::nullopt);
    nuevaEspecie.setFechaRevision(std::nullopt);

    // Toda ficha nueva nace borrador: se publica con POST /:id/publicar
    // cuando su autor la da por terminada.
    nuevaEspecie.setEstado(EspecieEstado::Borrador);
    nuevaEspecie.setPublicadoPor(std::nullopt);
    nuevaEspecie.setFechaPublicacion(std::nullopt);

    try {
      validarEspecie(nuevaEspecie);
    } catch (const std::invalid_argument& e) {
      json error = {{"error", e.what()}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto especieCreada = service->createEspecie(nuevaEspecie);
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Created, especieCreada.toJson().dump());

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

void EspecieController::update(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  auto identity = requireSesion(request, response);
  if (!identity) return;

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

    const auto especieExistente = service->getEspecieById(id);
    if (!especieExistente) {
      json error = {{"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    // Permiso sobre la categoría actual de la ficha, antes de validar el
    // cuerpo: quien no cura esta ficha se lleva un 403, no un 400.
    if (!requireCuradorDeCategoria(*identity, especieExistente->getCategoriaId(),
                                   response)) {
      return;
    }

    auto requestJson = json::parse(body);
    Especie especieActualizada = Especie::fromJson(requestJson);
    especieActualizada.setId(id);

    // Si el cuerpo mueve la ficha a otra categoría, también hace falta permiso
    // sobre la de destino: si no, un curador podría sacarla de su ámbito.
    if (especieActualizada.getCategoriaId() != especieExistente->getCategoriaId() &&
        !requireCuradorDeCategoria(*identity, especieActualizada.getCategoriaId(),
                                   response)) {
      return;
    }

    // Quien edita queda como revisor real; no se toma del body del cliente.
    especieActualizada.setRevisadoPor(identity->userId);

    try {
      validarEspecie(especieActualizada);
    } catch (const std::invalid_argument& e) {
      json error = {{"error", e.what()}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto especieActualizadaResult = service->updateEspecie(especieActualizada);
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok,
                  especieActualizadaResult.toJson().dump());

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

void EspecieController::updateFotos(const Pistache::Rest::Request& request,
                                    Pistache::Http::ResponseWriter response) {
  auto identity = requireSesion(request, response);
  if (!identity) return;

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

    // Las fotos son contenido de la ficha: mismo permiso que editarla.
    const auto especieExistente = service->getEspecieById(id);
    if (!especieExistente) {
      json error = {{"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }
    if (!requireCuradorDeCategoria(*identity, especieExistente->getCategoriaId(),
                                   response)) {
      return;
    }

    if (request.body().empty()) {
      json error = {{"error", "El cuerpo de la petición no puede estar vacío"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    const auto requestJson = json::parse(request.body());
    FotoKeysUpdate update;

    if (requestJson.contains("foto_portada_key")) {
      update.updateFotoPortadaKey = true;
      if (requestJson["foto_portada_key"].is_null()) {
        update.fotoPortadaKey = std::nullopt;
      } else if (requestJson["foto_portada_key"].is_string()) {
        update.fotoPortadaKey =
            requestJson["foto_portada_key"].get<std::string>();
      } else {
        throw std::invalid_argument("'foto_portada_key' debe ser string o null");
      }
    }

    if (requestJson.contains("fotos_keys")) {
      if (!requestJson["fotos_keys"].is_array()) {
        throw std::invalid_argument("'fotos_keys' debe ser un array JSON");
      }
      update.updateFotosKeys = true;
      for (const auto& item : requestJson["fotos_keys"]) {
        if (!item.is_string()) {
          throw std::invalid_argument("'fotos_keys' solo acepta strings");
        }
        update.fotosKeys.push_back(item.get<std::string>());
      }
    }

    const Especie updated = service->updateFotoKeys(id, update);
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, updated.toJson().dump());
  } catch (const json::parse_error& e) {
    json error = {{"error", "JSON inválido: " + std::string(e.what())}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::invalid_argument& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    if (std::string(e.what()) == "Especie no encontrada") {
      response.send(Pistache::Http::Code::Not_Found, error.dump());
    } else {
      response.send(Pistache::Http::Code::Service_Unavailable, error.dump());
    }
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void EspecieController::remove(const Pistache::Rest::Request& request,
                               Pistache::Http::ResponseWriter response) {
  auto identity = requireSesion(request, response);
  if (!identity) return;

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

    const auto especieExistente = service->getEspecieById(id);
    if (!especieExistente) {
      json error = {{"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }
    if (!requireCuradorDeCategoria(*identity, especieExistente->getCategoriaId(),
                                   response)) {
      return;
    }

    bool eliminado = service->deleteEspecie(id);
    if (!eliminado) {
      json error = {{"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    json success = {{"message", "Especie eliminada correctamente"}};
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

void EspecieController::cambiarEstado(const Pistache::Rest::Request& request,
                                      Pistache::Http::ResponseWriter& response,
                                      EspecieEstado destino) {
  auto identity = requireSesion(request, response);
  if (!identity) return;

  try {
    int id = 0;
    try {
      id = std::stoi(request.param(":id").as<std::string>());
    } catch (const std::exception&) {
      id = 0;
    }
    if (id <= 0) {
      json error = {{"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    // Publicar es una edición más de la ficha: mismo permiso por categoría.
    const auto especieExistente = service->getEspecieById(id);
    if (!especieExistente) {
      json error = {{"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }
    if (!requireCuradorDeCategoria(*identity, especieExistente->getCategoriaId(),
                                   response)) {
      return;
    }

    // El conflicto de estado se decide aquí para que el 409 no se confunda
    // con un 400 de validación de la ficha al publicar.
    const bool yaEnDestino =
        (destino == EspecieEstado::Publicada) != especieExistente->esBorrador();
    if (yaEnDestino) {
      json error = {{"error", destino == EspecieEstado::Publicada
                                  ? "La especie ya está publicada"
                                  : "La especie ya es un borrador"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Conflict, error.dump());
      return;
    }

    const Especie resultado =
        destino == EspecieEstado::Publicada
            ? service->publicarEspecie(id, identity->userId)
            : service->despublicarEspecie(id);

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, resultado.toJson().dump());

  } catch (const std::invalid_argument& e) {
    // Publicar revalida los atributos por reino: una ficha que no cumple el
    // schema no se hace pública, y eso es un 400.
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Bad_Request, error.dump());
  } catch (const std::out_of_range& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::runtime_error& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Not_Found, error.dump());
  } catch (const std::exception& e) {
    json error = {{"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void EspecieController::publicar(const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response) {
  cambiarEstado(request, response, EspecieEstado::Publicada);
}

void EspecieController::despublicar(const Pistache::Rest::Request& request,
                                    Pistache::Http::ResponseWriter response) {
  cambiarEstado(request, response, EspecieEstado::Borrador);
}

void EspecieController::searchByNombreCientifico(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto query = request.query();
    if (!query.has("nombre")) {
      json error = {{"success", false},
                    {"error", "El parámetro 'nombre' es obligatorio"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    std::string nombre = utils::percentDecode(query.get("nombre").value());
    if (nombre.empty()) {
      json error = {{"success", false},
                    {"error", "El parámetro 'nombre' no puede estar vacío"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    auto especie = service->searchByNombreCientifico(nombre);

    // Misma regla que getById: un borrador ajeno no aparece por búsqueda.
    if (especie && especie->esBorrador()) {
      const auto identity = extractIdentity(request);
      const bool puedeVerlo =
          identity && moderacion->puedeEditarCategoria(
                          identity->userId, identity->role,
                          especie->getCategoriaId());
      if (!puedeVerlo) especie = std::nullopt;
    }

    if (!especie) {
      json error = {{"success", false}, {"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    json successResponse = {{"success", true},
                            {"data", especie->toJson()},
                            {"message", "Especie encontrada exitosamente"}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, successResponse.dump());

  } catch (const std::exception& e) {
    json error = {{"success", false}, {"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void EspecieController::uploadImagen(const Pistache::Rest::Request& request,
                                     Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    auto principal_str = request.param(":principal").as<std::string>();
    int especie_id;
    bool es_principal;
    try {
      especie_id = std::stoi(id_str);
      es_principal = (principal_str == "true" || principal_str == "1");
    } catch (const std::exception& e) {
      json error = {{"success", false},
                    {"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    const size_t MAX_SIZE = 10 * 1024 * 1024;  // 10MB
    auto imagen_data = request.body();
    if (imagen_data.size() > MAX_SIZE) {
      json error = {
          {"success", false},
          {"error", "La imagen es demasiado grande. Máximo 10MB permitido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }
    if (imagen_data.empty()) {
      json error = {{"success", false},
                    {"error", "Los datos de la imagen no pueden estar vacíos"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    std::vector<uint8_t> imagen_vector(imagen_data.begin(), imagen_data.end());
    std::string imagen_url =
        service->subirImagen(especie_id, imagen_vector, es_principal);

    json success = {{"success", true},
                    {"message", "Imagen subida correctamente"},
                    {"url", imagen_url}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, success.dump());

  } catch (const std::exception& e) {
    json error = {{"success", false}, {"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void EspecieController::setImagenPrincipal(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    std::string imagen_url = request.param(":url").as<std::string>();
    int especie_id;

    try {
      especie_id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"success", false},
                    {"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    bool success = service->setImagenPrincipal(especie_id, imagen_url);
    if (!success) {
      json error = {
          {"success", false},
          {"error", "No se pudo establecer la imagen como principal"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    json successResponse = {
        {"success", true},
        {"message", "Imagen principal cambiada correctamente"},
        {"url", imagen_url}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, successResponse.dump());

  } catch (const std::exception& e) {
    json error = {{"success", false}, {"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void EspecieController::removeImagen(const Pistache::Rest::Request& request,
                                     Pistache::Http::ResponseWriter response) {
  try {
    std::string filename = request.param(":filename").as<std::string>();

    // Validar nombre de archivo para evitar path traversal attacks
    if (filename.find("..") != std::string::npos ||
        filename.find("/") != std::string::npos ||
        filename.find("\\") != std::string::npos) {
      json error = {{"success", false},
                    {"error", "Nombre de archivo inválido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    std::string filepath = "./static/images/especie/" + filename;

    if (!std::filesystem::exists(filepath)) {
      json error = {{"success", false}, {"error", "Imagen no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    // Construir URL de la imagen
    std::string image_url = "/api/images/especies/" + filename;

    // Intentar extraer el ID de especie del nombre del archivo
    int especie_id = -1;
    size_t pos = filename.find("especie_");
    if (pos != std::string::npos) {
      pos += 8;  // longitud de "especie_"
      size_t end_pos = filename.find("_", pos);
      if (end_pos != std::string::npos) {
        try {
          especie_id = std::stoi(filename.substr(pos, end_pos - pos));
        } catch (const std::exception& e) {
          // Si no se puede extraer el ID, continuar con la eliminación del
          // archivo
        }
      }
    }

    // Eliminar del servicio si tenemos el especie_id
    if (especie_id != -1) {
      service->removeImagenFromEspecie(especie_id, image_url);
    }

    // Eliminar archivo físico
    try {
      std::filesystem::remove(filepath);
    } catch (const std::filesystem::filesystem_error& e) {
      json error = {
          {"success", false},
          {"error", "Error al eliminar el archivo: " + std::string(e.what())}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
      return;
    }

    json success = {{"success", true},
                    {"message", "Imagen eliminada correctamente"},
                    {"filename", filename}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, success.dump());

  } catch (const std::exception& e) {
    json error = {{"success", false}, {"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

void EspecieController::getImagenesByEspecie(
    const Pistache::Rest::Request& request,
    Pistache::Http::ResponseWriter response) {
  try {
    auto id_str = request.param(":id").as<std::string>();
    int especie_id;

    try {
      especie_id = std::stoi(id_str);
    } catch (const std::exception& e) {
      json error = {{"success", false},
                    {"error", "ID debe ser un número válido"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Bad_Request, error.dump());
      return;
    }

    // Verificar que la especie existe
    auto especie = service->getEspecieById(especie_id);
    if (!especie) {
      json error = {{"success", false}, {"error", "Especie no encontrada"}};
      response.headers().add<Pistache::Http::Header::ContentType>(
          MIME(Application, Json));
      response.send(Pistache::Http::Code::Not_Found, error.dump());
      return;
    }

    auto imagenes = service->getImagenesByEspecieId(especie_id);

    json success = {{"success", true},
                    {"message", "Imágenes obtenidas correctamente"},
                    {"especie_id", especie_id},
                    {"imagenes", imagenes}};

    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, success.dump());

  } catch (const std::exception& e) {
    json error = {{"success", false}, {"error", e.what()}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}

// NUEVO MÉTODO PARA SERVIR IMÁGENES
void EspecieController::serveImage(const Pistache::Rest::Request& request,
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

    std::string filepath = "./static/images/especie/" + filename;

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

    // Establecer headers apropiados para la imagen
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Image, Jpeg));
    // Nota: Pistache puede no tener Header::CacheControl, usar header genérico
    Pistache::Http::Header::Raw cacheHeader("Cache-Control", "max-age=3600");
    response.headers().addRaw(cacheHeader);

    response.send(Pistache::Http::Code::Ok, content);

  } catch (const std::exception& e) {
    json error = {{"error", "Error interno del servidor"}};
    response.headers().add<Pistache::Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Pistache::Http::Code::Internal_Server_Error, error.dump());
  }
}
