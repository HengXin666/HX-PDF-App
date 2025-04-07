#include <net/HttpRequestFactory.h>

#include <QUrl>
#include <QtNetwork>

namespace HX {

HttpRequestFactory::HttpRequestFactory()
    : _manager(std::make_unique<QNetworkAccessManager>())
{}

HX::ReplyAsync HttpRequestFactory::get(const QString& url) && {
    return HX::ReplyAsync{
        _manager->get(QNetworkRequest{url}),
        std::move(_manager)
    };
}

HX::ReplyAsync HttpRequestFactory::range(const QString& url, int begin, int end) && {
    QNetworkRequest req{url};
    QByteArray rangeValue = QByteArray("bytes=")
                            + QByteArray::number(begin)
                            + "-" 
                            + QByteArray::number(end);
    req.setRawHeader("Range", rangeValue);
    return HX::ReplyAsync{
        _manager->get(req),
        std::move(_manager)
    };
}

HX::ReplyAsync HttpRequestFactory::useRangeGetSize(const QString& url) && {
    return HX::ReplyAsync{
        _manager->head(QNetworkRequest{url}),
        std::move(_manager)
    };
}

} // namespace HX