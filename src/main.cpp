#include <QApplication>
#include <window/MainWindow.h>

int _1main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}

//////////////////////////////////////////////////////////////////////

#include <fstream>

#include <mupdf/pdf.h>

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

struct MupdfRaii {
    explicit MupdfRaii(const char* filePath)
        : _ctx(fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT))
        , _ss(filePath)
        , _stream(fz_new_stream(_ctx, &_ss, streamNext, streamDrop))
    {
        _stream->seek = streamSeek;
        fz_register_document_handlers(_ctx);
        fz_try(_ctx) {
            _doc = fz_open_document_with_stream(_ctx, filePath, _stream);
            // _doc = fz_open_document(_ctx, filePath);
            int page_count = fz_count_pages(_ctx, _doc);
            qDebug() << "页数:" << page_count;
            fz_drop_document(_ctx, _doc);
        }
        fz_catch(_ctx) {
            qDebug() << "错误:" << fz_caught_message(_ctx);
        }
    }

    ~MupdfRaii() noexcept {
        fz_drop_document(_ctx, _doc);
        fz_drop_stream(_ctx, _stream);
        fz_drop_context(_ctx);
    }

    fz_context* _ctx;
    StreamState _ss;
    fz_stream* _stream;
    fz_document* _doc{};
};

int main() {
    const char* filename1 = "D:/command/Github/HX-PDF-App/TestPdfSrc/C++-Templates-The-Complete-Guide-zh-20220903.pdf";
    const char* filename2 = "D:/command/Github/HX-PDF-App/TestPdfSrc/imouto.epub";
    MupdfRaii pdf1{filename1};
    MupdfRaii pdf2{filename2};
}