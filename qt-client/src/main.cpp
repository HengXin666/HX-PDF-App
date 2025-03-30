#include <QApplication>
#include <window/MainWindow.h>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}

//////////////////////////////////////////////////////////////////////

#include <fstream>

#include <mupdf/pdf.h>

#include <mu/Document.h>

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

int _main() {
    const char* filename1 = "D:/command/Github/HX-PDF-App/cpp-backend/pdf-data/Cpp-T.pdf";
    const char* filename2 = "D:/command/Github/HX-PDF-App/qt-client/TestPdfSrc/imouto.epub";
    auto bs = HX::Mu::StreamFuncBuilder{
        [](fz_context* ctx, fz_stream* stm, size_t max) ->int {
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
        [](fz_context* ctx, fz_stream* stm, int64_t offset, int whence) -> void {
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

    HX::Mu::Document pdf2{filename2};
    pdf2.setStream({}).buildDocument(".epub");
    infoAll(pdf2);

    return 0;
}