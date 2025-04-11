#include <mu/stream/NetRangeStream.h>

#include <mupdf/fitz.h>

namespace HX::Mu {

NetRangeStream::NetRangeStream(const char* url) noexcept 
    : _url(url)
    , _cli()
    , _maxLen(0)
    , _nowPos(0)
    , _range({0, 0})
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
    // @todo 可行的优化
    // https://mupdf.readthedocs.io/en/latest/progressive-loading.html
    StreamFuncBuilder res{[](fz_context* ctx, fz_stream* stm, size_t max) -> int {
        auto* sp = (NetRangeStream*)stm->state;
        if (sp->_nowPos >= sp->_range.end || sp->_nowPos < sp->_range.begin) {

            sp->_range.begin = sp->_nowPos;
            sp->_range.end = std::min(sp->_nowPos + NetRangeStream::BufMaxSize, sp->_maxLen);
            sp->_cli.range(sp->_url, sp->_range.begin, sp->_range.end)
                    .exec([=](QNetworkReply* reply) {
                        sp->_buf = reply->readAll();
                        // qDebug() << QString{"Range: [%1, %2)"}
                        //     .arg(sp->_range.begin)
                        //     .arg(sp->_range.end) << sp->_buf.size();
                    });
        }

        size_t available = sp->_range.end - sp->_nowPos;
        size_t to_read = std::min(max, available);
        if (to_read == 0) {
            // qDebug() << "maxLen:" << sp->_maxLen << sp->_buf;
            return EOF;
        }

        stm->rp = reinterpret_cast<unsigned char*>(sp->_buf.data() + (sp->_nowPos - sp->_range.begin));
        stm->wp = stm->rp + to_read;
        sp->_nowPos += to_read;
        stm->pos = sp->_nowPos;

        return *stm->rp++;
    },
    [](fz_context* ctx, void* state) -> void {
        // 无需操作, 因为RAII
    },
    [](fz_context* ctx, fz_stream* stm, int64_t offset, int whence) -> void {
        auto* sp = (NetRangeStream*)stm->state;
        // qDebug() << "offset ->" << offset << "By" << whence;
        switch (whence) {
        case 0: sp->_nowPos = offset; break;
        case 1: sp->_nowPos += offset; break;
        case 2: sp->_nowPos = sp->_maxLen + offset; break;
        default: sp->_nowPos = offset; break;
        }
        // 重新获取当前的绝对位置, 并更新 stm->pos
        stm->pos = sp->_nowPos;
        stm->rp = stm->wp = nullptr;
    }};
    return res;
}

} // namespace HX::Mu