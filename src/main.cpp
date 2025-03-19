#include <QApplication>
#include <QMainWindow>
#include <QPdfView>
#include <QPdfDocument>
#include <QVBoxLayout>

class PdfViewer : public QMainWindow {
public:
    PdfViewer() {
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        
        // 创建 PDF 视图组件
        pdfView = new QPdfView(this);
        layout->addWidget(pdfView);

        // 设置 PDF 阅读器的整体行为
        // QPdfView::PageMode::SinglePage 一次显示一页
        // QPdfView::PageMode::MultiPage  允许滚动浏览文档中的所有页面
        pdfView->setPageMode(QPdfView::PageMode::MultiPage);

        // 加载本地 PDF
        pdfDocument = new QPdfDocument(this);
        pdfDocument->load("D:/command/小组/C++-Templates-The-Complete-Guide-zh-20220903.pdf");  // 你的本地 PDF 文件
        pdfView->setDocument(pdfDocument);

        setCentralWidget(centralWidget);
        resize(800, 600);
    }

private:
    QPdfView *pdfView;
    QPdfDocument *pdfDocument;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PdfViewer viewer;
    viewer.show();
    return app.exec();
}
