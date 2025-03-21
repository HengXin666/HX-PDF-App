#include <QApplication>
#include <window/MainWindow.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}