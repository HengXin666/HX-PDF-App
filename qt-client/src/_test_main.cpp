#include <QApplication>
#include <window/MainWindow.h>

int _12main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}

//////////////////////////////////////////////////////////////////////

#include <fstream>

#include <QLabel>

#include <mupdf/pdf.h>

#include <mu/Document.h>
#include <mu/Page.h>

struct StreamState {
    explicit StreamState(const char* filePath)
        : _in(std::ifstream{filePath, std::ios::in | std::ios::binary})
    {}

    bool isOpen() const {
        return _in.is_open();
    }

    int read(size_t max) {
        return _in.read((char *)buf, max).gcount();
    }

    bool isEof() const {
        return _in.eof();
    }

    auto pos() {
        return _in.tellg();
    }

    void seekg(int64_t offset, int whence) {
        _in.seekg(offset, whence);
    }

    inline static constexpr std::size_t MaxBufSize = 4096;

    unsigned char buf[MaxBufSize]{};

    ~StreamState() noexcept {
        _in.close();
    }

    std::ifstream _in;
};

/**
 * @brief 读取数据
 * @param ctx 
 * @param stm [in, out] [rp, wp) 之间的数据是有效的, 如果有数据, 会更新 [rp, wq) 指向新的数据, 并且返回 *stm->rp++
 * @param max 期望读取的最大字节数
 * @return int 没有数据则返回-1, 否则返回读取到的字节数
 */
int streamNext(fz_context* ctx, fz_stream* stm, size_t max) {
    auto* sp = (StreamState*)stm->state;
    int res = sp->read(std::min(max, StreamState::MaxBufSize));
    if (res == 0 && sp->isEof()) {
        return -1;
    }
    stm->rp = &sp->buf[0];
    stm->wp = &sp->buf[res];
    stm->pos += res;
    return *stm->rp++;
}

/**
 * @brief 随机读取
 * @param ctx 
 * @param stm 
 * @param offset 
 * @param whence 
 */
void streamSeek(fz_context* ctx, fz_stream* stm, int64_t offset, int whence) {
    auto* sp = (StreamState*)stm->state;
    // 根据 whence 转换为正确的 std::ios_base::seekdir
    std::ios_base::seekdir dir;
    switch (whence) {
        case 0: dir = std::ios::beg; break;
        case 1: dir = std::ios::cur; break;
        case 2: dir = std::ios::end; break;
        default: dir = std::ios::beg; break;
    }
    sp->seekg(offset, static_cast<int>(dir));
    // 重新获取当前的绝对位置, 并更新 stm->pos
    stm->pos = sp->pos();
}

/**
 * @brief 清理流的内部状态
 * @param ctx 
 * @param state 
 */
void streamDrop(fz_context* ctx, void* state) {
    // 无需操作, 因为RAII
    qDebug() << "raii";
}


struct MuPdf : public HX::Mu::Document {
    explicit MuPdf(const char* filePath)
        : HX::Mu::Document(filePath)
        , _ss(filePath)
    {}


    Document& setStream(HX::Mu::StreamFuncBuilder const& builder) override {
        _stream = fz_new_stream(_ctx, &_ss, builder._next, builder._drop);
        _stream->seek = builder._seek;
        return *this;
    }

    ~MuPdf() noexcept override {

    }

private:
    StreamState _ss;
};

class TextRenderWidget : public QWidget {
    Q_OBJECT

public:
    explicit TextRenderWidget(QWidget *parent = nullptr);
    void setTextItems(const std::vector<HX::Mu::TextItem>& items); // 设置文本数据

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::vector<HX::Mu::TextItem> textItems; // 存储文本
};

TextRenderWidget::TextRenderWidget(QWidget *parent) : QWidget(parent) {}

void TextRenderWidget::setTextItems(const std::vector<HX::Mu::TextItem>& items) {
    textItems = items;
    update(); // 触发重绘
}

#include <QPainter>

void TextRenderWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.fillRect(rect(), Qt::white);

    for (const auto& item : textItems) {
        painter.setFont(item.font);
        painter.setPen(item.color);
        // painter.drawRect(item.rect);
        painter.drawText(item.origin, item.text);
    }
}

#include <QBuffer>
#include <QImageReader>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTransform>
#include <QScreen>

int _2main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    const char* filename1 = "D:/command/Github/HX-PDF-App/cpp-backend/pdf-data/Cpp-T.pdf";
    const char* filename2 = "D:/command/Github/HX-PDF-App/qt-client/TestPdfSrc/imouto.epub";
    auto bs = HX::Mu::StreamFuncBuilder{
        [](fz_context* ctx, fz_stream* stm, size_t max) -> int {
            auto* sp = (StreamState*)stm->state;
            int res = sp->read(std::min(max, StreamState::MaxBufSize));
            if (res == 0 && sp->isEof()) {
                return -1;
            }
            stm->rp = &sp->buf[0];
            stm->wp = &sp->buf[res];
            stm->pos += res;
            return *stm->rp++;
        },
        [](fz_context* ctx, void* state) -> void {
            // 无需操作, 因为RAII
            qDebug() << "raii";
        },
        [](fz_context* ctx, fz_stream* stm, int64_t offset,
           int whence) -> void {
            auto* sp = (StreamState*)stm->state;
            // 根据 whence 转换为正确的 std::ios_base::seekdir
            std::ios_base::seekdir dir;
            switch (whence) {
            case 0: dir = std::ios::beg; break;
            case 1: dir = std::ios::cur; break;
            case 2: dir = std::ios::end; break;
            default: dir = std::ios::beg; break;
            }
            sp->seekg(offset, static_cast<int>(dir));
            // 重新获取当前的绝对位置, 并更新 stm->pos
            qDebug() << stm->pos << "->" << offset;
            stm->pos = sp->pos();
        }
        /*
        int64_t pos = stm->pos - (stm->wp - stm->rp);

            // Convert to absolute pos
            if (whence == 1) {
                // Was relative to current pos
                offset += pos;
            } else if (whence == 2) {
                // Was relative to end
                offset += stm->pos;
            }

            if (offset < 0)
                offset = 0;
            if (offset > stm->pos)
                offset = stm->pos;
            stm->rp += (int)(offset - pos);
        */
    };
    MuPdf pdf1{filename1};
    pdf1.setStream(bs).buildDocument(".pdf");

    auto infoAll = [](HX::Mu::Document const& pdf) {
        qDebug() << "页码:" << pdf.pageCount();
        qDebug() << "Title:" << pdf.title();
        qDebug() << "author:" << pdf.author();
        qDebug() << "subject:" << pdf.subject();
        qDebug() << "keywords:" << pdf.keywords();
        qDebug() << "creator:" << pdf.creator();
        qDebug() << "producer:" << pdf.producer();
        qDebug() << "creationDate:" << pdf.creationDate();
        qDebug() << "modDate:" << pdf.modDate();
    };

    infoAll(pdf1);

    MuPdf pdf2{filename2};
    pdf2.setStream(bs).buildDocument(".epub");
    // qDebug() << "页码:" << pdf2.pageCount();

    auto img = pdf2.page(10)->renderImage(300);
    auto label = new QLabel;
    // QTransform transform;
    // double scaleFactor = 1;
    // transform.scale(scaleFactor, scaleFactor);
    img.setDevicePixelRatio(2);
    auto pixmap = QPixmap::fromImage(
        img
        // .transformed(transform, Qt::SmoothTransformation)
    );
    qDebug() << img.size();
    // pixmap.setDevicePixelRatio(2.0);
    label->setBaseSize(img.size());
    label->setPixmap(pixmap);
    label->show();


    QMainWindow mainWindow;
    auto *renderWidget = new TextRenderWidget();

    mainWindow.resize(800, 600);
    mainWindow.show();

    for (int i = 0; i < 100; ++i) {
        renderWidget->setTextItems(pdf2.page(i)->renderText(1));
        mainWindow.setCentralWidget(renderWidget);
    }

    qDebug() << QGuiApplication::primaryScreen()->logicalDotsPerInch();

    return app.exec();
}

#include <widget/MuMainWidget.h>

int _main(int argc, char* argv[]) {    
    QApplication app(argc, argv);
    const char* filename1 = "D:/command/Github/HX-PDF-App/cpp-backend/pdf-data/Cpp-T.pdf";
    const char* filename2 = "http://127.0.0.1:28205/files/github.pdf";
    const char* filename3 = "http://127.0.0.1:28205/files/Cpp-T.pdf";
    QScrollArea* scrollArea = new QScrollArea;
    auto* muWidget = new HX::MuMainWidget{scrollArea};
    muWidget->setDocument(filename3);
    scrollArea->setWidget(muWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->show();
    return app.exec();
}

#include <net/HttpClient.h>

int _23main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    const char* url = "http://www.baidu.com/";
    HX::HttpClient cli{};

    cli.range("http://127.0.0.1:28205/files/Cpp-T.pdf", 0, 1)
        .exec([&](QNetworkReply* reply) {
            qDebug() << reply->headers();
            qDebug() << reply->readAll();

            cli.range("http://127.0.0.1:28205/files/Cpp-T.pdf", 1, 2)
                .exec([&](QNetworkReply* reply) {
                    qDebug() << reply->headers();
                    qDebug() << reply->readAll();

                    cli.range("http://127.0.0.1:28205/files/Cpp-T.pdf", 2, 3)
                        .exec([&](QNetworkReply* reply) {
                            qDebug() << reply->headers();
                            qDebug() << reply->readAll();
                        });
                });
        });

    // HX::HttpRequestFactory{}
    //     .get(url)
    //     .async([](QNetworkReply* reply) {
    //         qDebug() << "succes:" <<
    //         reply->header(QNetworkRequest::ContentLengthHeader);
    //     });
    qDebug() << "AUV";
    return app.exec();
}

#include "_test_main.moc"