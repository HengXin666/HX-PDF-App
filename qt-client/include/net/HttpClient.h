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
#ifndef _HX_HTTP_CLIENT_H_
#define _HX_HTTP_CLIENT_H_

#include <QObject>

#include <memory>

#include <net/ReplyAsync.hpp>

namespace HX {

class [[nodiscard]] HttpClient : public QObject {
    Q_OBJECT
public:
    explicit HttpClient(QObject* parent = nullptr);

    HX::ReplyAsync get(const QString& url) &&;

    HX::ReplyAsync range(const QString& url, int begin, int end) &&;

    HX::ReplyAsync range(const QString& url) &&;
private:
    HttpClient& operator=(HttpClient&&) = delete;
    std::unique_ptr<QNetworkAccessManager> _manager;
};

} // namespace HX

#endif // !_HX_HTTP_CLIENT_H_