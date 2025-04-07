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
public:
    static StreamFuncBuilder make();

    explicit NetRangeStream(const char* url) noexcept;

    void init();
private:
    QString _url;
    HX::HttpClient _cli;
    std::size_t _maxLen;
    std::size_t _nowPos;
    QByteArray _buf;
};

} // namespace HX::Mu

#endif // !_HX_NET_RANGE_STREAM_H_