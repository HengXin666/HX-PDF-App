#include <widget/ViewManageWidget.h>

#include <QVBoxLayout>
#include <QStackedWidget>

#include <singleton/GlobalSingleton.hpp>

HX::ViewManageWidget::ViewManageWidget(QWidget* parent)
    : QWidget(parent)
    , _stackedWidget(new QStackedWidget{this})
{
    (new QVBoxLayout{this})->addWidget(_stackedWidget);
    HX::GlobalSingleton::get().viewManageProxy.setMainDisplayBar(this);
}