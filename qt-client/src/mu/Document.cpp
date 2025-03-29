#include <mu/Document.h>

#include <stdexcept>

#include <mupdf/fitz.h>

namespace HX::Mu {
    
Document::Document(const char* filePath) noexcept
    : _ctx(fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT))
    , _stream()
    , _doc()
{
    fz_register_document_handlers(_ctx);
}


Document& Document::setStream(StreamFuncBuilder const&) {
    return *this;
}

void Document::buildDocument(const char* filePath) {
    fz_try(_ctx) {
        _doc = _stream 
            ? fz_open_document_with_stream(_ctx, filePath, _stream)
            : fz_open_document(_ctx, filePath);
    } fz_catch(_ctx) {
        throw std::runtime_error{fz_caught_message(_ctx)};
    }
}

int Document::pageCount() const {
    int page_count = -1;
    fz_try(_ctx) {
        page_count = fz_count_pages(_ctx, _doc);
    } fz_catch(_ctx) {
        throw std::runtime_error{fz_caught_message(_ctx)};
    }
    return page_count;
}

Document::~Document() noexcept {
    if (_doc)
        fz_drop_document(_ctx, _doc);
    if (_stream)
        fz_drop_stream(_ctx, _stream);
    if (_ctx)
        fz_drop_context(_ctx);
}

} // namespace HX::HX::Mu