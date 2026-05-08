#include "../../include/repository/postgresql_especie_repository.hpp"
#include <stdexcept>
#include <iostream>

PostgreSQLEspecieRepository::PostgreSQLEspecieRepository(std::shared_ptr<Database> database)
    : database(database)
{ // Cambiado de db a database para consistencia
    initDatabase();
}

void PostgreSQLEspecieRepository::initDatabase()
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Crear la tabla si no existe
        txn.exec(
            "CREATE TABLE IF NOT EXISTS especies ("
            "id SERIAL PRIMARY KEY, "
            "nombre_cientifico VARCHAR(100) NOT NULL UNIQUE, "
            "nombre_comun VARCHAR(100), "
            "descripcion TEXT, "
            "habitat TEXT, "
            "distribucion TEXT, "
            "endemica BOOLEAN DEFAULT FALSE, "
            "genero_id INTEGER REFERENCES generos(id), " // Corregida la sintaxis de FOREIGN KEY
            "estado_conservacion VARCHAR(50)"
            ");"
            "CREATE TABLE IF NOT EXISTS especies_imagenes ("
            "id SERIAL PRIMARY KEY,"
            "especie_id INTEGER REFERENCES especies(id) ON DELETE CASCADE,"
            "url VARCHAR(255) NOT NULL,"
            "es_principal BOOLEAN DEFAULT FALSE,"
            "UNIQUE(especie_id, url)"
            ");");

        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al inicializar la base de datos: " << e.what() << std::endl;
        throw;
    }
}

Especie PostgreSQLEspecieRepository::mapRowToEspecie(const pqxx::row &row)
{
    Especie especie(
        row["id"].as<int>(),
        row["nombre_cientifico"].as<std::string>(),
        row["nombre_comun"].as<std::string>(),
        row["genero_id"].as<int>(), // Cambiado de genero (string) a genero_id (int)
        row["descripcion"].as<std::string>(),
        row["habitat"].as<std::string>(),
        row["distribucion"].as<std::string>(),
        row["endemica"].as<bool>(),
        row["estado_conservacion"].as<std::string>());
        auto imagenes = getImagenes(especie.getId());
        for (const auto& url : imagenes) {
            especie.addImagenUrl(url);
        }
        return especie;
}

std::vector<Especie> PostgreSQLEspecieRepository::getAll()
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec("SELECT * FROM especies ORDER BY nombre_cientifico");
        std::vector<Especie> especies;

        for (const auto &row : result)
        {
            especies.push_back(mapRowToEspecie(row));
        }

        return especies;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al obtener todas las especies: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Especie> PostgreSQLEspecieRepository::findById(int id)
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT * FROM especies WHERE id = $1",
            id);

        if (result.empty())
        {
            return std::nullopt;
        }

        return mapRowToEspecie(result[0]);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al obtener especie por ID: " << e.what() << std::endl;
        throw;
    }
}

std::optional<Especie> PostgreSQLEspecieRepository::getByNombreCientifico(const std::string &nombre)
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT * FROM especies WHERE nombre_cientifico ILIKE $1",
            "%" + nombre + "%");

        // Return the first matching result as an optional
        if (result.size() > 0)
        {
            return mapRowToEspecie(result[0]);
        }
        else
        {
            return std::nullopt;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al buscar especies por nombre científico: " << e.what() << std::endl;
        throw;
    }
}

std::vector<Especie> PostgreSQLEspecieRepository::getByGenero(const std::string &nombre)
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "SELECT * FROM especies,generos WHERE genero_id=generos.id AND generos.nombre= $1",
            nombre);

        std::vector<Especie> especies;
        for (const auto &row : result)
        {
            especies.push_back(mapRowToEspecie(row));
        }

        return especies;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al buscar especies por género: " << e.what() << std::endl;
        throw;
    }
}

Especie PostgreSQLEspecieRepository::create(const Especie &especie)
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Validar nombre científico único
        pqxx::result check = txn.exec_params(
            "SELECT COUNT(*) FROM especies WHERE nombre_cientifico = $1",
            especie.getNombreCientifico());

        if (check[0][0].as<int>() > 0)
        {
            throw std::invalid_argument("El nombre científico ya existe en la base de datos");
        }

        pqxx::result result = txn.exec_params(
            "INSERT INTO especies (nombre_cientifico, nombre_comun, descripcion, "
            "habitat, distribucion, endemica, genero_id, estado_conservacion) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7, $8) RETURNING *",
            especie.getNombreCientifico(),
            especie.getNombreComun(),
            especie.getDescripcion(),
            especie.getHabitat(),
            especie.getDistribucion(),
            especie.isEndemica(),
            especie.getGeneroId(),
            especie.getEstadoConservacion());

        txn.commit();
        return mapRowToEspecie(result[0]);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al crear especie: " << e.what() << std::endl;
        throw;
    }
}

Especie PostgreSQLEspecieRepository::update(const Especie &especie)
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Verificar si existe la especie
        pqxx::result check = txn.exec_params(
            "SELECT COUNT(*) FROM especies WHERE id = $1",
            especie.getId());

        if (check[0][0].as<int>() == 0)
        {
            throw std::invalid_argument("Especie no encontrada con ID: " + std::to_string(especie.getId()));
        }
        // Validar nombre científico único (excepto para la misma especie)
        check = txn.exec_params(
            "SELECT COUNT(*) FROM especies WHERE nombre_cientifico = $1 AND id != $2",
            especie.getNombreCientifico(),
            especie.getId());

        if (check[0][0].as<int>() > 0)
        {
            throw std::invalid_argument("El nombre científico ya existe para otra especie");
        }

        pqxx::result result = txn.exec_params(
            "UPDATE especies SET nombre_cientifico = $1, nombre_comun = $2, genero_id = $3, "
            "descripcion = $4, habitat = $5, distribucion = $6, endemica = $7, "
            " estado_conservacion = $8 WHERE id = $9 RETURNING id",
            especie.getNombreCientifico(),
            especie.getNombreComun(),
            especie.getGeneroId(),
            especie.getDescripcion(),
            especie.getHabitat(),
            especie.getDistribucion(),
            especie.isEndemica(),
            especie.getEstadoConservacion(),
            especie.getId());

        txn.commit();
        return mapRowToEspecie(result[0]);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al actualizar especie: " << e.what() << std::endl;
        throw; // Relanza la excepción
    }
}

bool PostgreSQLEspecieRepository::remove(int id)
{
    try
    {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        pqxx::result result = txn.exec_params(
            "DELETE FROM especies WHERE id = $1 RETURNING id",
            id);

        txn.commit();
        return !result.empty();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error al eliminar especie: " << e.what() << std::endl;
        throw;
    }
}

bool PostgreSQLEspecieRepository::agregarImagen(int especie_id, const std::string& imagen_url, bool es_principal) {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);

    try {
        // Si es principal, desmarca las demás
        if (es_principal) {
            txn.exec_params(
                "UPDATE especies_imagenes SET es_principal = false "
                "WHERE especie_id = $1", especie_id
            );
        }

        txn.exec_params(
            "INSERT INTO especies_imagenes (especie_id, url, es_principal) "
            "VALUES ($1, $2, $3)",
            especie_id, imagen_url, es_principal
        );

        txn.commit();
        return true;
    } catch (const std::exception& e) {
        txn.abort();
        return false;
    }
}

std::vector<std::string> PostgreSQLEspecieRepository::getImagenes(int especie_id) {
    auto conn = database->createConnection();
    pqxx::read_transaction txn(*conn);

    std::vector<std::string> imagenes;
    auto result = txn.exec_params(
        "SELECT url FROM especies_imagenes "
        "WHERE especie_id = $1 ORDER BY es_principal DESC",
        especie_id
    );

    for (const auto& row : result) {
        imagenes.push_back(row["url"].as<std::string>());
    }

    return imagenes;
}

bool PostgreSQLEspecieRepository::eliminarImagen(int especie_id, const std::string& imagen_url) {
    auto conn = database->createConnection();
    pqxx::work txn(*conn);
    try {
        pqxx::work txn(*conn);
        auto result = txn.exec_params(
            "DELETE FROM especie_imagenes WHERE especie_id = $1 AND imagen_url = $2",
            especie_id, imagen_url
        );
        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception& e) {
        return false;
    }
}

bool PostgreSQLEspecieRepository::setImagenPrincipal(int especie_id, const std::string& imagen_url) {
    try {
        auto conn = database->createConnection();
        pqxx::work txn(*conn);

        // Desmarcar todas las imágenes como no principales
        txn.exec_params(
                "UPDATE especies_imagenes SET es_principal = false WHERE especie_id = $1",
                especie_id
        );

        // Marcar la imagen especificada como principal
        auto result = txn.exec_params(
                "UPDATE especies_imagenes SET es_principal = true "
                "WHERE especie_id = $1 AND url = $2",
                especie_id, imagen_url
        );

        txn.commit();
        return result.affected_rows() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error al establecer imagen principal: " << e.what() << std::endl;
        return false;
    }
}