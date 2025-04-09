#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HX-PDF-App.
 *
 * HX-PDF-App is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HX-PDF-App is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HX-PDF-App.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _HX_NET_RANGE_STREAM_H_
#define _HX_NET_RANGE_STREAM_H_

#include <mu/StreamFuncBuilder.hpp>
#include <net/HttpClient.h>

namespace HX::Mu {

class NetRangeStream {
    inline static constexpr std::size_t BufMaxSize = 4 * 1024 * 1024;
public:
    static StreamFuncBuilder make();

    explicit NetRangeStream(const char* url) noexcept;

    void init();
private:
    QString _url;
    HX::HttpClient _cli;
    std::size_t _maxLen;
    std::size_t _nowPos;

    // 当前所属范围: 采用的是绝对范围, 即同`_nowPos`等价取值为 [0, _maxLen)
    struct Range {
        std::size_t begin;
        std::size_t end;
    } _range;

    // 取值示例: [_nowPos - begin, _nowPos - end]
    QByteArray _buf;
};

} // namespace HX::Mu

#endif // !_HX_NET_RANGE_STREAM_H_