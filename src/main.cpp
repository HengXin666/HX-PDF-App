#include <QApplication>
#include <window/MainWindow.h>

int _main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}

#include <ts.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWin;
    Sidebar sidebar;
    mainWin.addSidebar(&sidebar);
    mainWin.show();
    return app.exec();
}
