#include <mu/stream/NetRangeStream.h>

#include <mupdf/fitz.h>

namespace HX::Mu {

NetRangeStream::NetRangeStream(const char* url) noexcept 
    : _url(url)
    , _cli()
    , _maxLen(0)
    , _nowPos(0)
    , _buf()
{}

void NetRangeStream::init() {
    _maxLen = _cli
        .useRangeGetSize(_url)
        .exec([](QNetworkReply* reply){
        if (reply->error() != QNetworkReply::NetworkError::NoError) [[unlikely]] {
            throw std::runtime_error{reply->errorString().toUtf8()};
        }
        return reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
    });
}

StreamFuncBuilder NetRangeStream::make() {
    StreamFuncBuilder res{[](fz_context* ctx, fz_stream* stm, size_t max) -> int {
        auto* sp = (NetRangeStream*)stm->state;
        int res = sp->_cli.range(sp->_url, sp->_nowPos, sp->_nowPos + max)
            .exec([=](QNetworkReply* reply) {
                sp->_buf = reply->readAll();
                return sp->_buf.size() - 1;
            });

        stm->rp = reinterpret_cast<unsigned char*>(&sp->_buf[0]);
        stm->wp = reinterpret_cast<unsigned char*>(&sp->_buf[res]);
        stm->pos += res;
        return *stm->rp++;
    },
    [](fz_context* ctx, void* state) -> void {
        // 无需操作, 因为RAII
    },
    [](fz_context* ctx, fz_stream* stm, int64_t offset, int whence) -> void {
        auto* sp = (NetRangeStream*)stm->state;
        switch (whence) {
        case 0: sp->_nowPos = offset; break;
        case 1: sp->_nowPos += offset; break;
        case 2: sp->_nowPos = sp->_maxLen + offset; break;
        default: sp->_nowPos = offset; break;
        }
        // 重新获取当前的绝对位置, 并更新 stm->pos
        stm->pos = sp->_nowPos;
    }};
    return res;
}

} // namespace HX::Mu