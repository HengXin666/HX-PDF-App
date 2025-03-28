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

// bui 流

struct BuildMuStream {
    using NextFunPtr = int (*)(fz_context* ctx, fz_stream* stm, size_t max);
    using DropFunPtr = void (*)(fz_context* ctx, void* state);
    using SeekFunPtr = void (*)(fz_context* ctx, fz_stream* stm, int64_t offset, int whence);

    BuildMuStream(
        NextFunPtr next,
        DropFunPtr drop,
        SeekFunPtr seek
    )
        : _next(next)
        , _drop(drop)
        , _seek(seek)
    {}

    NextFunPtr _next;
    DropFunPtr _drop;
    SeekFunPtr _seek;
};

struct MuPdf {
    explicit MuPdf(const char* filePath)
        : _ctx(fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT))
        , _ss(filePath)
        , _stream()
    {
        fz_register_document_handlers(_ctx);
    }

    MuPdf& setStream(BuildMuStream const& bui) {
        _stream = fz_new_stream(_ctx, &_ss, bui._next, bui._drop);
        _stream->seek = bui._seek;
        return *this;
    }

    void buildDocument(const char* magic) {
        fz_try(_ctx) {
            _doc = fz_open_document_with_stream(_ctx, magic, _stream);
        } fz_catch(_ctx) {
            throw std::runtime_error{fz_caught_message(_ctx)};
        }
    }

    int pageCnt() {
        int page_count = -1;
        fz_try(_ctx) {
            page_count = fz_count_pages(_ctx, _doc);
            qDebug() << "页数:" << page_count;
        } fz_catch(_ctx) {
            throw std::runtime_error{fz_caught_message(_ctx)};
        }
        return page_count;
    }

    ~MuPdf() noexcept {
        fz_drop_document(_ctx, _doc);
        fz_drop_stream(_ctx, _stream);
        fz_drop_context(_ctx);
    }

private:
    // 禁止拷贝赋值和拷贝构造
    MuPdf(MuPdf const&) = delete;
    MuPdf& operator=(MuPdf const&) = delete;

    fz_context* _ctx;
    StreamState _ss;
    fz_stream* _stream;
    fz_document* _doc;
};

int main() {
    const char* filename1 = "D:/command/Github/HX-PDF-App/TestPdfSrc/C++-Templates-The-Complete-Guide-zh-20220903.pdf";
    const char* filename2 = "D:/command/Github/HX-PDF-App/TestPdfSrc/imouto.epub";
    auto bs = BuildMuStream{
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
    pdf1.pageCnt();

    MuPdf pdf2{filename2};
    pdf2.setStream({
        streamNext,
        streamDrop,
        streamSeek,
    }).buildDocument(".epub");
    pdf2.pageCnt();
}