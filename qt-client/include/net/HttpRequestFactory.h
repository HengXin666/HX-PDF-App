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
#ifndef _HX_HTTP_REQUEST_FACTORY_H_
#define _HX_HTTP_REQUEST_FACTORY_H_

#include <memory>

#include <net/ReplyAsync.hpp>

namespace HX {

/**
 * @brief Http请求工厂类
 */
class [[nodiscard]] HttpRequestFactory {
public:
    explicit HttpRequestFactory();

    HX::ReplyAsync get(const QString& url) &&;

    HX::ReplyAsync range(const QString& url, int begin, int end) &&;

    /**
     * @brief 使用`Range`获取待传输的文件大小
     * @param url 
     * @return HX::ReplyAsync 应使用`reply->header(QNetworkRequest::ContentLengthHeader)`获取文件大小
     */
    HX::ReplyAsync useRangeGetSize(const QString& url) &&;
private:
    HttpRequestFactory& operator=(HttpRequestFactory&&) = delete;
    std::unique_ptr<QNetworkAccessManager> _manager;
};

} // namespace HX

#endif // !_HX_HTTP_REQUEST_FACTORY_H_