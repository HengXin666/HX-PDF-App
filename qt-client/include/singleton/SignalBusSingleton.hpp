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
#ifndef _HX_SIGNAL_BUS_SINGLETON_H_
#define _HX_SIGNAL_BUS_SINGLETON_H_

#include <QObject>
#include <QMutex>

namespace HX {

/**
 * @brief 信号总线单例
 * 
 * 信号总线使用方法:
 *      点击某控件 --发出信号--> 信号总线 --分发信号--> 绑定了信号总线的信号的控件 --> 变化
 * 
 * 示例:
 *      [点击: 音乐暂停按钮] --发出信号--> [信号总线] 
 *      --分发信号--> [所有有关的控件]:
 *                      --> 1) 音频播放: 停止
 *                      --> 2) 音频播放图标: 切换
 *                      --> 3) ...
 */
class SignalBusSingleton : public QObject {
    Q_OBJECT

    explicit SignalBusSingleton() noexcept {}
    SignalBusSingleton& operator=(SignalBusSingleton&&) = delete;
public:
    /**
     * @brief `线程安全`获取信号总线单例
     * @return SignalBusSingleton& 
     */
    inline static SignalBusSingleton& get() {
        static QMutex mutex{};
        QMutexLocker _{&mutex};
        static SignalBusSingleton singleton{};
        return singleton;
    }

Q_SIGNALS:
    /**
     * @brief 退出程序信号
     */
    void exitProgramSignaled();

    /**
     * @brief 根窗口最大化状态切换信号, 如果当前为最大化则`mainWindowShowNormalSignaled`
     * 反之, 则发出`mainWindowShowMaximizeSignaled`信号
     */
    void mainWindowMaximizeSwitched();

    /**
     * @brief 根窗口最小化信号
     */
    void mainWindowShowMinimizeSignaled();

    /**
     * @brief 根窗口最大化信号
     */
    void mainWindowShowMaximizeSignaled();

    /**
     * @brief 根窗口大小恢复常态(最大化之前的大小)
     */
    void mainWindowShowNormalSignaled();
};

} // namespace HX

#endif // !_HX_SIGNAL_BUS_SINGLETON_H_