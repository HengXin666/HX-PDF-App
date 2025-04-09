#include <net/HttpClient.h>

#include <QUrl>
#include <QtNetwork>

namespace HX {

QNetworkReply* HttpClient::_get(const QString& url) {
    return _manager->get(QNetworkRequest{url});
}

QNetworkReply* HttpClient::_range(const QString& url, int begin, int end) {
    QNetworkRequest req{url};
    QByteArray rangeValue = QByteArray("bytes=")
                            + QByteArray::number(begin)
                            + "-" 
                            + QByteArray::number(end - 1);
    req.setRawHeader("Range", rangeValue);
    return _manager->get(req);
}

QNetworkReply* HttpClient::_useRangeGetSize(const QString& url) {
    return _manager->head(QNetworkRequest{url});
}

} // namespace HX