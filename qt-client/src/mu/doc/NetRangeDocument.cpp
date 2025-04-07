#include <mu/doc/NetRangeDocument.h>

#include <mupdf/fitz.h>

namespace HX::Mu {

NetRangeDocument::NetRangeDocument(const char* url) noexcept
    : HX::Mu::Document(url)
    , _nrs(url)
{}

Document& NetRangeDocument::setStream(StreamFuncBuilder const& builder) {
    _stream = fz_new_stream(_ctx, &_nrs, builder._next, builder._drop);
    _stream->seek = builder._seek;
    _nrs.init();
    return *this;
}

} // namespace HX::Mu