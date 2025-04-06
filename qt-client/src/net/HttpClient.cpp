#include <net/HttpClient.h>

#include <QUrl>
#include <QtNetwork>
#include <QEventLoop>

namespace HX {

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
    , _manager(std::make_unique<QNetworkAccessManager>())
{}

HX::ReplyAsync HttpClient::get(const QString& url) && {
    return HX::ReplyAsync{
        _manager->get(QNetworkRequest{url}),
        std::move(_manager)
    };
}

HX::ReplyAsync HttpClient::range(const QString& url, int begin, int end) && {
    QNetworkRequest req{url};

    return HX::ReplyAsync{
        _manager->get(req),
        std::move(_manager)
    };
}

HX::ReplyAsync HttpClient::range(const QString& url) && {
    return HX::ReplyAsync{
        _manager->head(QNetworkRequest{url}),
        std::move(_manager)
    };
}

} // namespace HX