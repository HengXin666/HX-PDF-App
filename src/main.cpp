#include <QApplication>
#include <QMainWindow>
#include <QPdfView>
#include <QPdfDocument>
#include <QVBoxLayout>

#include <qpdf/QPDF.hh>

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

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <QDebug>
#include <iostream>
#include <map>

void parsePDF(const std::string &filename) {
    QPDF pdf;
    try {
        pdf.processFile(filename.c_str());
        
        // 获取页数
        std::vector<QPDFObjectHandle> pages = pdf.getAllPages();
        qDebug() << "PDF 总页数:" << pages.size();

        // 读取 PDF 元数据
        QPDFObjectHandle info = pdf.getTrailer().getKey("/Info");
        if (info.isDictionary()) {
            for (auto const& key : info.getKeys()) {
                qDebug() << key << ":" << info.getKey(key).unparse();
            }
        }

        // 获取交叉引用表信息
        qDebug() << "交叉引用表 (XRef Table):" << pdf.getXRefTable().size();
    } catch (std::exception &e) {
        std::cerr << "解析失败: " << e.what() << std::endl;
    }
}

int main() {
    parsePDF("D:/command/小组/C++-Templates-The-Complete-Guide-zh-20220903.pdf");  // 本地 PDF 文件
    return 0;
}

int _main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    PdfViewer viewer;
    viewer.show();
    return app.exec();
}
