#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>

class Sidebar : public QWidget {
    Q_OBJECT
    bool visible = true;
    QPropertyAnimation *animation;

public:
    Sidebar(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedWidth(200);
        setStyleSheet("background-color: #3498db;");

        animation = new QPropertyAnimation(this, "geometry");
        animation->setDuration(300);

        QVBoxLayout *layout = new QVBoxLayout(this);
        QPushButton *toggleBtn = new QPushButton("Toggle Sidebar", this);
        layout->addWidget(toggleBtn);
        connect(toggleBtn, &QPushButton::clicked, this, &Sidebar::toggle);
    }

    void toggle() {
        int startX = visible ? 0 : -width();
        int endX = visible ? -width() : 0;

        animation->setStartValue(QRect(startX, y(), width(), height()));
        animation->setEndValue(QRect(endX, y(), width(), height()));
        animation->start();

        visible = !visible;
    }

    bool isSidebarVisible() const { return visible; } // 新增方法，返回当前 Sidebar 状态
};

class MainWindow : public QMainWindow {
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // 主布局（垂直方向）
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setSpacing(0);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        // 顶部工具栏
        QWidget *topBar = new QWidget(this);
        topBar->setFixedHeight(50);
        topBar->setStyleSheet("background-color: #2ecc71;");
        mainLayout->addWidget(topBar);

        // 主要内容区域
        QWidget *mainContentWidget = new QWidget(this);
        QHBoxLayout *contentLayout = new QHBoxLayout(mainContentWidget);
        contentLayout->setSpacing(0);
        contentLayout->setContentsMargins(0, 0, 0, 0);

        // 主要显示区域
        QPushButton *content = new QPushButton("Main Content", this);
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        contentLayout->addWidget(content);

        mainLayout->addWidget(mainContentWidget);

        // 创建 Sidebar，并设置为 mainContentWidget 的子控件
        sidebar = new Sidebar(mainContentWidget);

        // 让 Sidebar 初始隐藏
        sidebar->setGeometry(-sidebar->width(), 0, sidebar->width(), mainContentWidget->height());
    }

    void resizeEvent(QResizeEvent *) override {
        if (sidebar) {
            int sidebarX = sidebar->isSidebarVisible() ? 0 : -sidebar->width();
            sidebar->setGeometry(sidebarX, 0, sidebar->width(), centralWidget()->height() - 50);
        }
    }

private:
    Sidebar *sidebar;
};