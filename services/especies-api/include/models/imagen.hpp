
#ifndef IMAGEN_HPP
#define IMAGEN_HPP

#include <string>


class Imagen {
private:
    int id;
    std::string url;
    bool es_principal;
public:
    Imagen(int id, const std::string &url, bool es_principal)
            : id(id), url(url), es_principal(es_principal) {}

    // Getters
    int getId() const { return id; }

    const std::string &getUrl() const { return url; }

    bool getEsPrincipal() const { return es_principal; }

    // Setters
    void setId(int newId) { id = newId; }

    void setUrl(const std::string &newUrl) { url = newUrl; }

    void setEsPrincipal(bool newEsPrincipal) { es_principal = newEsPrincipal; }

};


#endif //IMAGEN_HPP
