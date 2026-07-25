// Copyright 2025 <Obrero/Obrero>
#include <pistache/endpoint.h>
#include <pistache/router.h>

#include <iostream>
#include <memory>

#include "../include/controllers/especie_controller.hpp"
#include "../include/controllers/avistamiento_controller.hpp"
#include "../include/controllers/categoria_moderacion_controller.hpp"
#include "../include/controllers/familia_controller.hpp"
#include "../include/controllers/genero_controller.hpp"
#include "../include/controllers/upload_controller.hpp"
#include "../include/repository/postgres_categoria_moderacion_repository.hpp"
#include "../include/repository/postgres_familia_repository.hpp"
#include "../include/repository/postgres_genero_repository.hpp"
#include "../include/repository/postgresql_especie_repository.hpp"
#include "../include/repository/postgres_avistamiento_repository.hpp"
#include "../include/services/avistamiento_service.hpp"
#include "../include/services/categoria_moderacion_service.hpp"
#include "../include/services/especie_service.hpp"
#include "../include/services/familia_service.hpp"
#include "../include/services/genero_service.hpp"
#include "../include/services/upload_presign_service.hpp"
#include "../include/utils/atributos_schema_validator.hpp"
#include "../include/utils/database.hpp"
#include "../include/utils/config.hpp"

int main(int argc, char** argv) {
  // Get port from environment or use default
	Config config;
	int port = config.getApiPort();
	std::string host = config.getApiHost();

	Pistache::Address addr(host, Pistache::Port(port));


  // Setup connection string para PostgreSQL
  std::string connectionString =
      "host=" +
      std::string(std::getenv("DB_HOST") ? std::getenv("DB_HOST")
                                         : "localhost") +
      " port=" +
      std::string(std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "5432") +
      " dbname=" +
      std::string(std::getenv("DB_NAME") ? std::getenv("DB_NAME")
                                         : "chiloe_flora") +
      " user=" +
      std::string(std::getenv("DB_USER") ? std::getenv("DB_USER")
                                         : "postgres") +
      " password=" +
      std::string(std::getenv("DB_PASSWORD") ? std::getenv("DB_PASSWORD")
                                             : "postgres");

  printf("Connection string: %s\n", connectionString.c_str());
  auto dataBase = std::make_shared<Database>(connectionString);

  // Initialize repositories
  auto familiaRepository =
      std::make_shared<PostgresFamiliaRepository>(dataBase);
  auto generoRepository = std::make_shared<PostgresGeneroRepository>(dataBase);
  auto especieRepository =
      std::make_shared<PostgreSQLEspecieRepository>(dataBase);
  auto avistamientoRepository =
      std::make_shared<PostgresAvistamientoRepository>(dataBase);
  auto categoriaModeracionRepository =
      std::make_shared<PostgresCategoriaModeracionRepository>(dataBase);

  // El schema lo gestiona scripts/migrate.sh contra la BD antes de arrancar
  // el binario (ver services/especies-api/migrations/README.md).

  // Cargar JSON Schemas de atributos por reino. Si falla, abortar arranque:
  // un servicio sin validadores aceptaría JSONB arbitrario y rompería el
  // contrato del schema multi-reino.
  const std::string schemasDir =
      std::getenv("SCHEMAS_DIR") ? std::getenv("SCHEMAS_DIR")
                                  : "/etc/chiloe-especies-api/schemas";
  auto schemaValidator =
      std::make_shared<AtributosSchemaValidator>(schemasDir);
  std::cout << "JSON Schemas cargados desde: " << schemasDir << std::endl;

  // Initialize services
  auto familiaService = std::make_shared<FamiliaService>(familiaRepository);
  auto generoService = std::make_shared<GeneroService>(generoRepository);
  auto uploadPresignService = std::make_shared<UploadPresignService>();
  auto especieService =
      std::make_shared<EspecieService>(especieRepository, schemaValidator,
                                       uploadPresignService);
  auto avistamientoService =
      std::make_shared<AvistamientoService>(avistamientoRepository,
                                            uploadPresignService);
  auto categoriaModeracionService =
      std::make_shared<CategoriaModeracionService>(categoriaModeracionRepository);

  // Initialize controllers
  auto familiaController = std::make_shared<FamiliaController>(familiaService);
  auto generoController = std::make_shared<GeneroController>(generoService);
  auto especieController = std::make_shared<EspecieController>(
      especieService, categoriaModeracionService);
  auto avistamientoController =
      std::make_shared<AvistamientoController>(avistamientoService);
  auto uploadController =
      std::make_shared<UploadController>(uploadPresignService);
  auto categoriaModeracionController =
      std::make_shared<CategoriaModeracionController>(categoriaModeracionService);

  // Setup router
  auto router = std::make_shared<Pistache::Rest::Router>();

  // Health check endpoint
  Pistache::Rest::Routes::Get(*router, "/health",
                              [](const Pistache::Rest::Request& request,
                                 Pistache::Http::ResponseWriter response)
                                  -> Pistache::Rest::Route::Result {
                                response.send(Pistache::Http::Code::Ok, "OK");
                                return Pistache::Rest::Route::Result::Ok;
                              });

  // Setup routes
  FamiliaController::setupRoutes(*router, familiaController);
  GeneroController::setupRoutes(*router, generoController);
  EspecieController::setupRoutes(*router, especieController);
  AvistamientoController::setupRoutes(*router, avistamientoController);
  UploadController::setupRoutes(*router, uploadController);
  CategoriaModeracionController::setupRoutes(*router, categoriaModeracionController);

  // Configure server - MEJORADO para red local
  Pistache::Http::Endpoint server(addr);
  auto opts =
      Pistache::Http::Endpoint::options()
          .threads(4)  // Number of threads
          .flags(Pistache::Tcp::Options::ReuseAddr |
                 Pistache::Tcp::Options::ReusePort)  // Añadido ReusePort
          .maxRequestSize(
              50 * 1024 *
              1024);  // 50MB máximo por request (reemplaza maxPayload)

  server.init(opts);
  server.setHandler(router->handler());

  // Start server - MEJORADO el mensaje
  std::cout << "==================================" << std::endl;
  std::cout << "Server starting on port " << port << std::endl;
  std::cout << "Local access: http://localhost:" << port << std::endl;
  std::cout << "Network access: http://[YOUR_LOCAL_IP]:" << port << std::endl;
  std::cout << "Health check: http://localhost:" << port << "/health"
            << std::endl;
  std::cout << "==================================" << std::endl;

  server.serve();

  return 0;
}
