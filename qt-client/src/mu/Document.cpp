#include <mu/Document.h>

#include <stdexcept>

#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
#include <mupdf/pdf/object.h>

#include <mu/Page.h>

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
    if (_pageCnt != -1) {
        return _pageCnt;
    }
    fz_try(_ctx) {
        _pageCnt = fz_count_pages(_ctx, _doc);
    } fz_catch(_ctx) [[unlikely]] {
        throw std::runtime_error{fz_caught_message(_ctx)};
    }
    return _pageCnt;
}

std::shared_ptr<Page> Document::page(int index) {
    if (!_doc || index < 0 || index >= _pageCnt) [[unlikely]] {
        throw _doc 
            ? std::runtime_error{"Page number is illegal."} 
            : std::runtime_error{"Please load the document first."};
    }
    auto& res = _pageList[index];
    if (res) {
        return res;
    }
    return res = std::make_shared<Page>(*this, index);
}

bool Document::needsPassword() const {
    if (!_doc) [[unlikely]] {
        // 请先加载文档
        throw std::runtime_error{"Please load the document first."};
    }
    return fz_needs_password(_ctx, _doc);
}

bool Document::authPassword(const QString& password) {
    if (!_doc) [[unlikely]] {
        // 请先加载文档
        throw std::runtime_error{"Please load the document first."};
    }
    return fz_authenticate_password(_ctx, _doc, password.toLocal8Bit().data());
}

QString Document::title() const {
    return info("Title");
}

QString Document::author() const {
    return info("Author");
}

QString Document::subject() const {
    return info("Subject");
}

QString Document::keywords() const {
    return info("Keywords");
}

QString Document::creator() const {
    return info("Creator");
}

QString Document::producer() const {
    return info("Producer");
}

QDateTime Document::creationDate() const {
    QString str = info("CreationDate");
    if (str.isEmpty()) {
        return QDateTime{};
    }
    return QDateTime::fromString(str.left(16),
            "'D:'yyyyMMddHHmmss");
}

QDateTime Document::modDate() const {
    QString str = info("ModDate");
    if (str.isEmpty()) {
        return QDateTime{};
    }
    return QDateTime::fromString(str.left(16),
            "'D:'yyyyMMddHHmmss");
}

Document::~Document() noexcept {
    if (_doc)
        fz_drop_document(_ctx, _doc);
    if (_stream)
        fz_drop_stream(_ctx, _stream);
    if (_ctx)
        fz_drop_context(_ctx);
}

QString Document::info(const char* key) const {
    pdf_document* xref = (pdf_document *)_doc;
    pdf_obj* info = pdf_dict_gets(_ctx, pdf_trailer(_ctx, xref), (char*)"Info");
    if (!info)
        return QString{};
    pdf_obj* obj = pdf_dict_gets(_ctx, info, (char*)key);
    if (!obj)
        return QString{};
    char* str = pdf_new_utf8_from_pdf_string_obj(_ctx, obj);
    QString res = QString::fromUtf8(str);
    free(str);
    return res;
}

} // namespace HX::HX::Mu