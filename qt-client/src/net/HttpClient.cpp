#include <net/HttpClient.h>

#include <QUrl>
#include <QtNetwork>
#include <QEventLoop>

namespace HX {

HttpClient::HttpClient(QObject* parent)
    : QObject(parent)
    , _tcp(new QTcpSocket{this})
    , _headsBuf()
    , _resBuf()
{
    // 检测服务器是否回复了数据
    connect(_tcp, &QTcpSocket::readyRead, [this]() {
        // 接收服务器发送的数据
        QByteArray recvMsg = _tcp->readAll();
        qDebug() << recvMsg;
        _resBuf += recvMsg;

        // @todo 解析 res 即可
        emit parseCompleted();
    });

    // 检测是否和服务器是否连接成功了
    connect(_tcp, &QTcpSocket::connected, this, [this]() {
        qDebug() << "连接服务器成功!!!";
    });

    // 检测服务器是否和客户端断开了连接
    connect(_tcp, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "服务器已经断开了连接, ...";
    });
}

void HttpClient::connectToHost(const QString& url) {
    QUrl qurl{url};
    _tcp->connectToHost(qurl.host(), qurl.port());
}

QString HttpClient::get(const QString& path) {
    QString buf;
    buf += "GET ";
    buf += path;
    buf += " HTTP/1.1\r\n";
    buf += std::move(_headsBuf);
    buf += "\r\n";
    _tcp->write(buf.toUtf8());

    // 异步转同步
    QEventLoop loop;
    QObject::connect(this, &HttpClient::parseCompleted, &loop,
                     &QEventLoop::quit);
    return std::move(_resBuf);
}

} // namespace HX