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
#ifndef _HX_REPLY_ASYNC_H_
#define _HX_REPLY_ASYNC_H_

#include <QEventLoop>
#include <QNetworkReply>

namespace HX {

class ReplyAsync {
public:
    explicit ReplyAsync(QNetworkReply* reply)
        : _reply(reply)
        , _manger()
    {}

    explicit ReplyAsync(QNetworkReply* reply, std::unique_ptr<QNetworkAccessManager>&& manger)
        : _reply(reply)
        , _manger(std::move(manger))
    {}

    /**
     * @brief 异步回调, 注意需要处理正确和错误的情况, 因为绑定的是`QNetworkReply::finished`信号
     * @param func 
     * @throw std::runtime_error 没有资源所有权
     */
    void async(std::function<void(QNetworkReply*)> func) {
        if (!_manger) [[unlikely]] {
            // 没有资源所有权
            throw std::runtime_error{"No ownership of resources! (ReplyAsync::_manger is nullptr)"};
        }
        QObject::connect(_reply, &QNetworkReply::finished, _reply,
                         [manger = std::move(_manger), func = std::move(func),
                          reply = _reply] {
                             func(reply);
                             reply->disconnect(reply);
                         });
    }

    /**
     * @brief 同步等待, 注意需要处理正确和错误的情况, 因为绑定的是`QNetworkReply::finished`信号
     * @tparam Func [](QNetworkReply*) -> Res {}
     * @tparam Res 
     * @param func 
     * @throw std::runtime_error 不应该持有资源
     * @return Res 
     */
    template <typename Func, 
              typename Res = decltype(std::declval<Func>()(static_cast<QNetworkReply*>(nullptr)))>
    Res exec(Func&& func) {
        if (_manger) [[unlikely]] {
            // 不应该持有资源
            throw std::runtime_error{
                "Should not hold resources! (ReplyAsync::_manger is not nullptr)"
            };
        }
        QEventLoop loop;
        QObject::connect(_reply, &QNetworkReply::finished, &loop,
                         &QEventLoop::quit);
        loop.exec();
        return func(_reply);
    }

private:
    QNetworkReply* _reply;
    std::unique_ptr<QNetworkAccessManager> _manger;
};

} // namespace HX

#endif // !_HX_REPLY_ASYNC_H_