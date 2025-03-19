#include <view/HomeView.h>

#include <widget/TopBar.h>

HX::HomeView::HomeView(QWidget* parent)
    : QWidget(parent)
{
    HX::TopBar* _topBal = new HX::TopBar(this);
}