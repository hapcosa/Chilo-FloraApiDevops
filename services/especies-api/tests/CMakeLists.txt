# Tests unitarios (gtest). Se construyen solo con -DBUILD_TESTS=ON para no
# arrastrar GoogleTest en la imagen de producción.
#
# Cubren lógica pura sin dependencias de red ni BD:
#   - reino.cpp: serialización/parseo del enum Reino.
#   - atributos_schema_validator.cpp: validación de atributos_especificos por
#     reino contra los JSON Schemas reales de config/schemas/.
#   - categoria_moderacion.cpp: formato de slug y (de)serialización JSON.
#   - moderacion_service.cpp: quién puede editar qué categoría, con un
#     repositorio en memoria.
#   - postulacion_curador.cpp: parseo del cuerpo del postulante y reglas que no
#     delegamos en la BD.
#   - identificacion.cpp: parseo del cuerpo del identificador y la regla que
#     deriva el grado del avistamiento (quórum de 2/3 y voto decisivo del
#     curador), aislada de la BD a propósito.
#   - especie_estado.cpp + especie.cpp: estado editorial de la ficha y qué
#     acepta fromJson (el cliente no publica mandando "estado" en el cuerpo).

find_package(GTest REQUIRED)

add_executable(unit_tests
        test_reino.cpp
        test_atributos_schema_validator.cpp
        test_upload_presign_service.cpp
        test_categoria_moderacion.cpp
        test_moderacion_service.cpp
        test_postulacion_curador.cpp
        test_especie_estado.cpp
        test_identificacion.cpp
        ${CMAKE_SOURCE_DIR}/src/models/reino.cpp
        ${CMAKE_SOURCE_DIR}/src/models/especie_estado.cpp
        ${CMAKE_SOURCE_DIR}/src/models/especie.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/atributos_schema_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/services/upload_presign_service.cpp
        ${CMAKE_SOURCE_DIR}/src/models/categoria_moderacion.cpp
        ${CMAKE_SOURCE_DIR}/src/services/moderacion_service.cpp
        ${CMAKE_SOURCE_DIR}/src/models/postulacion_curador.cpp
        ${CMAKE_SOURCE_DIR}/src/models/identificacion.cpp
)

target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# El validador carga los schemas desde disco; le pasamos la ruta real del
# repo para que los tests usen exactamente los mismos archivos que producción.
target_compile_definitions(unit_tests PRIVATE
        SCHEMAS_DIR="${CMAKE_SOURCE_DIR}/config/schemas")

target_link_libraries(unit_tests
        GTest::gtest_main
        nlohmann_json_schema_validator
        OpenSSL::Crypto
        ${CURL_LIBRARIES}
)

add_test(NAME unit_tests COMMAND unit_tests)
