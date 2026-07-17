#ifndef UPLOAD_CONTROLLER_HPP
#define UPLOAD_CONTROLLER_HPP

#include <memory>

#include <pistache/http.h>
#include <pistache/router.h>

#include "../services/upload_presign_service.hpp"

class UploadController {
private:
    std::shared_ptr<UploadPresignService> service;

public:
    explicit UploadController(std::shared_ptr<UploadPresignService> service);

    void presignPut(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter response);

    static void setupRoutes(Pistache::Rest::Router& router,
                            std::shared_ptr<UploadController> controller) {
        using namespace Pistache::Rest;

        Routes::Post(router, "/api/v1/uploads/presign",
                     Routes::bind(&UploadController::presignPut, controller));
    }
};

#endif  // UPLOAD_CONTROLLER_HPP
