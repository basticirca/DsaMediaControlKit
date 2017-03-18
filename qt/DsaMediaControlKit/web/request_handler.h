#ifndef WEB_REQUEST_HANDLER_H
#define WEB_REQUEST_HANDLER_H

#include "httprequesthandler.h"
#include "controllers/preset_controller.h"

namespace Web {

class RequestHandler : public stefanfrings::HttpRequestHandler {
    Q_OBJECT
public:
    RequestHandler(QObject* parent=0);
    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

private:
    PresetController* preset_controller_;
};

} // namespace Web

#endif // WEB_REQUEST_HANDLER_H