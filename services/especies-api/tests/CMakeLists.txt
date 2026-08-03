# Tests unitarios (gtest). Se construyen solo con -DBUILD_TESTS=ON para no
# arrastrar GoogleTest en la imagen de producción.
#
# Cubren lógica pura sin dependencias de red ni BD:
#   - reino.cpp: serialización/parseo del enum Reino.
#   - atributos_schema_validator.cpp: validación de atributos_especificos por
#     reino contra los JSON Schemas reales de config/schemas/.
#   - categoria_moderacion.cpp: formato de slug y (de)serialización JSON.

find_package(GTest REQUIRED)

add_executable(unit_tests
        test_reino.cpp
        test_atributos_schema_validator.cpp
        test_upload_presign_service.cpp
        test_categoria_moderacion.cpp
        ${CMAKE_SOURCE_DIR}/src/models/reino.cpp
        ${CMAKE_SOURCE_DIR}/src/utils/atributos_schema_validator.cpp
        ${CMAKE_SOURCE_DIR}/src/services/upload_presign_service.cpp
        ${CMAKE_SOURCE_DIR}/src/models/categoria_moderacion.cpp
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
