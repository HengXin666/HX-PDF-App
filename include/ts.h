#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>

class Sidebar : public QWidget {
    Q_OBJECT
    bool visible = true;
    QPropertyAnimation *animation;

public:
    Sidebar(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedWidth(200); // 侧边栏固定宽度
        setStyleSheet("background-color: #3498db;");
        
        animation = new QPropertyAnimation(this, "geometry");
        animation->setDuration(300);
        
        QVBoxLayout *layout = new QVBoxLayout(this);
        QPushButton *toggleBtn = new QPushButton("Toggle Sidebar", this);
        layout->addWidget(toggleBtn);
        connect(toggleBtn, &QPushButton::clicked, this, &Sidebar::toggle);
    }

    void toggle() {
        QRect startGeom = geometry();
        QRect endGeom = visible ? QRect(-width(), y(), width(), height()) : QRect(0, y(), width(), height());
        
        animation->setStartValue(startGeom);
        animation->setEndValue(endGeom);
        animation->start();
        
        visible = !visible;
    }
};

class MainWindow : public QMainWindow {
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        QPushButton *content = new QPushButton("Main Content", this);
        mainLayout->addWidget(content);
    }

    void resizeEvent(QResizeEvent *) override {
        if (sidebar) {
            sidebar->setGeometry(0, 0, sidebar->width(), height());
        }
    }

    void addSidebar(Sidebar *s) {
        sidebar = s;
        sidebar->setParent(this);
        sidebar->setGeometry(0, 0, sidebar->width(), height());
    }

private:
    Sidebar *sidebar = nullptr;
};