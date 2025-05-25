#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>
#include <QObject>

#include <qmlModel/QmlPdfPageModel.h>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    // 强制使用 Basic 样式
    QQuickStyle::setStyle("Basic");
    QQmlApplicationEngine engine;
    engine.addImageProvider("pdf", HX::QmlPdfPageModel::get()); // 会自己析构

    // 注册单例
    qmlRegisterSingletonInstance(
        "QmlPdfPageModel", // 导入时候的名称 (import Xxx) (注意得是大写开头)
        1, 0, // 主版本号 与 次版本号
        "QmlPdfPageModel", // qml中使用的组件名称 (注意得是大写开头)
        HX::QmlPdfPageModel::get() // 单例对象指针
    );

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );
    // 应该使用 _ 和 [0-9a-Z], 不能使用`-`
    engine.loadFromModule("HX-PDF-App", "Main");

    return app.exec();
}