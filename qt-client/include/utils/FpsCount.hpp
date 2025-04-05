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
#ifndef _HX_FPS_COUNT_H_
#define _HX_FPS_COUNT_H_

#include <chrono>
#include <optional>

namespace HX {

class FpsCount {
public:
    explicit FpsCount() = default;

    std::optional<int> count() {
        ++_cnt;
        auto now  = std::chrono::steady_clock::now();
        auto ms_d = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - _start).count();
        if (ms_d < 1000) {
            return {};
        }
        _start = now;
        auto res = _cnt * 1000 / ms_d;
        _cnt = 0;
        return res;
    }

private:
    std::size_t _cnt = 0;
    decltype(std::chrono::steady_clock::now()) 
        _start = std::chrono::steady_clock::now();
};

} // namespace HX

#endif // !_HX_FPS_COUNT_H_