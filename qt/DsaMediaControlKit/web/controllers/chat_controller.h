#ifndef WEB_CHAT_CONTROLLER_H
#define WEB_CHAT_CONTROLLER_H

#include "httprequesthandler.h"

#include <QMultiMap>

#include "web/response.h"
#include "web/app/chat_message_model.h"

namespace Web {

/**
 * @brief The ChatController class parses and handles incoming chat requests
 * forwarded by an instance of Web::RequestHandler (see request_handler.h)
 */
class ChatController : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
private:
    /**
     * @brief The FunctionIndex enum references function names
     * by int.
     */
    enum FunctionIndex {
        NONE,
        MESSAGE
    };

    /**
     * @brief converts given function index to string.
     * @param idx: function index to convert
     * @return converted string
     */
    static QString const toString(FunctionIndex idx);

    /**
     * @brief converts given function index to bytearray.
     * @param idx: function index to convert
     * @return converted string
     */
    static QByteArray const toByteArray(FunctionIndex idx);

    /**
     * @brief converts given string to function index.
     * @param str: string to convert
     * @return function index converted.
     */
    static FunctionIndex toIndex(const QString& str);

    /**
     * @brief converts given byte array to function index.
     * @param arr: byte array to convert
     * @return function index converted.
     */
    static FunctionIndex toIndex(const QByteArray& arr);

public:
    ChatController(QObject* parent=0);

    /**
     * @brief returns the ChatMessageModel of this instance.
     */
    App::ChatMessageModel* getModel();

    /**
     * @brief Evaluates forwarded requests and replies using response.
     * @param request: instance encoding the request forwarded.
     * @param response: instance the response is written to.
     */
    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

private:
    /**
     * @brief writes not implemented error to response.
     * @param response: instance the response is written to.
     */
    void respondNotImplemented(stefanfrings::HttpResponse& response);

    /**
     * @brief Parses parameter list to let host receive message from
     * client (player).
     * Writes success of message forwarding.
     * @param parameters: List of request key, value parameters
     * @param response: instance the response is written to.
     */
    void parseMessageRequest(const QMultiMap<QByteArray,QByteArray>& parameters,
                             stefanfrings::HttpResponse& response);

    /**
     * @brief writes given Web::Response message to given response instance.
     * Additionally produces log of given response.
     * @param response_msg: instance used to produce formatted response message.
     * @param response: instance to write response to.
     * @param reply_state: state for response.write(...) call.
     */
    void writeAndLog(const Response& response_msg,
                     stefanfrings::HttpResponse& response, bool reply_state);

    App::ChatMessageModel* message_model_;
};

} // namespace Web

#endif // WEB_CHAT_CONTROLLER_H