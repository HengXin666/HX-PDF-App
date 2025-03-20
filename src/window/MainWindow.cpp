#include <window/MainWindow.h>

#include <QEvent>
#include <QHBoxLayout>

#include <singleton/GlobalSingleton.hpp>
#include <singleton/SignalBusSingleton.hpp>
#include <widget/ViewManageWidget.h>
#include <view/HomeView.h>

HX::MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("HX-PDF-APP");
    resize(800, 600);
    setLayout(new QHBoxLayout);
    layout()->addWidget(new HX::ViewManageWidget{this});
    GlobalSingleton::get().viewManageProxy.pushView(new HX::HomeView{});

    if (QWidget* mainWindow = this->window()) { // 获取主窗口
        mainWindow->installEventFilter(this);
    }

    connect(&HX::SignalBusSingleton::get(), &HX::SignalBusSingleton::exitProgramSignaled, this,
    [this]() {
        close();
    });
    connect(&HX::SignalBusSingleton::get(), &HX::SignalBusSingleton::mainWindowShowMinimizeSignaled, this,
    [this]() {
        showMinimized();
    });
    connect(&HX::SignalBusSingleton::get(), &HX::SignalBusSingleton::mainWindowMaximizeSwitched, this,
    [this]() {
        if (isMaximized()) {
            showNormal();
            HX::SignalBusSingleton::get().mainWindowShowNormalSignaled();
        } else {
            showMaximized();
            HX::SignalBusSingleton::get().mainWindowShowMaximizeSignaled();
        }
    });
}

bool HX::MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (QWidget* mainWindow = window(); obj == mainWindow && event->type() == QEvent::WindowStateChange) {
        if (isMaximized()) {
            HX::SignalBusSingleton::get().mainWindowShowMaximizeSignaled();
        } else {
            HX::SignalBusSingleton::get().mainWindowShowNormalSignaled();
        }
    }
    return QWidget::eventFilter(obj, event);
}