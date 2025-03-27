#include <QApplication>
#include <window/MainWindow.h>

#include <mupdf/pdf.h>

int _1main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}
#include <iostream>
#include <fstream>
#include <vector>
#include "mupdf/fitz.h"

struct CustomStream {
    std::ifstream file;
    unsigned char buffer[4096];
    size_t buffer_pos = 0;
    size_t buffer_len = 0;
};

static void custom_seek(fz_context* ctx, fz_stream* stm, int64_t offset, int whence) {
    CustomStream* stream = static_cast<CustomStream*>(stm->state);
    std::ios::seekdir dir;
    switch (whence) {
        case SEEK_SET: dir = std::ios::beg; break;
        case SEEK_CUR: dir = std::ios::cur; break;
        case SEEK_END: dir = std::ios::end; break;
        default: return;
    }

    stream->file.clear();
    stream->file.seekg(static_cast<std::streamoff>(offset), dir);

    // 边界检查
    if (stream->file.fail()) {
        stream->file.clear();
        stream->file.seekg(0, std::ios::end);
        std::streampos end_pos = stream->file.tellg();
        std::cerr << "Seek failed! File size: " << end_pos << std::endl;
        return;
    }

    std::streampos pos = stream->file.tellg();
    stm->pos = (pos == std::streampos(-1)) ? 0 : static_cast<int64_t>(pos);

    stream->buffer_pos = 0;
    stream->buffer_len = 0;
    stm->rp = stm->wp = stream->buffer;
    stm->avail = 0;

    std::cout << "Seek to offset=" << offset << ", whence=" << whence << ", new_pos=" << stm->pos << std::endl;
}

static void custom_drop(fz_context* ctx, void* state) {
    CustomStream* stream = static_cast<CustomStream*>(state);
    if (stream->file.is_open()) stream->file.close();
    delete stream;
}

static int custom_next(fz_context* ctx, fz_stream* stm, size_t max) {
    CustomStream* stream = static_cast<CustomStream*>(stm->state);

    if (stream->buffer_pos >= stream->buffer_len) {
        stream->file.clear();
        stream->file.read(reinterpret_cast<char*>(stream->buffer), sizeof(stream->buffer));
        stream->buffer_len = stream->file.gcount();
        stream->buffer_pos = 0;

        std::cout << "Read " << stream->buffer_len << " bytes, pos=" << stream->file.tellg() << std::endl;

        if (stream->buffer_len == 0) {
            stm->rp = stm->wp = stream->buffer;
            stm->avail = 0;
            return EOF;
        }
    }

    stm->rp = stream->buffer + stream->buffer_pos;
    stm->wp = stream->buffer + stream->buffer_len;
    stm->avail = stream->buffer_len - stream->buffer_pos;

    int byte = *stm->rp++;
    stream->buffer_pos++;
    stm->avail--;

    return byte;
}

int _sbmain() {
    const char* filename = "D:/command/Github/HX-PDF-App/TestPdfSrc/C++-Templates-The-Complete-Guide-zh-20220903.pdf";

    fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
    if (!ctx) {
        std::cerr << "无法创建MuPDF上下文" << std::endl;
        return 1;
    }
    fz_register_document_handlers(ctx);

    CustomStream* stream = new CustomStream();
    stream->file.open(filename, std::ios::binary);
    if (!stream->file.is_open()) {
        std::cerr << "无法打开文件" << std::endl;
        delete stream;
        fz_drop_context(ctx);
        return 1;
    }

    // 初始化文件指针到开头
    stream->file.seekg(0, std::ios::beg);
    stream->file.clear();

    fz_stream* fz_stream = fz_new_stream(ctx, stream, custom_next, custom_drop);
    fz_stream->seek = custom_seek;

    fz_try(ctx) {
        fz_document* doc = fz_open_document_with_stream(ctx, ".pdf", fz_stream);
        int page_count = fz_count_pages(ctx, doc);
        std::cout << "页数: " << page_count << std::endl;
        fz_drop_document(ctx, doc);
    }
    fz_catch(ctx) {
        std::cerr << "错误: " << fz_caught_message(ctx) << std::endl;
    }

    fz_drop_stream(ctx, fz_stream);
    fz_drop_context(ctx);
    return 0;
}

struct MupdfRaii {
    explicit MupdfRaii()
        : _ctx(fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT))
    {}

    ~MupdfRaii() noexcept {
        fz_drop_context(_ctx);
    }

    fz_context* _ctx;
    fz_stream* _stream;
    fz_document* _doc;
};

int main() {
    fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
    fz_drop_context(ctx);
}