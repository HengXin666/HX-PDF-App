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
#ifndef _HX_RANGE_REQUEST_CACHE_MANAGEMENT_H_
#define _HX_RANGE_REQUEST_CACHE_MANAGEMENT_H_

#include <optional>
#include <vector>
#include <stdexcept>

#include <QByteArray>

namespace HX {

/**
 * @brief 范围请求缓存管理
 */
class RangeRequestCacheManagement {
    // 一块的大小 (256 KB)
    inline static constexpr std::size_t BlockMaxSize = 256 * 1024;
public:
    explicit RangeRequestCacheManagement(std::size_t maxLen) noexcept 
        : _maxLen(maxLen)
        , _blockArr((maxLen - 1) / BlockMaxSize + 1) // 上取整
    {}

    struct RangeRequestCache {
        QByteArray& bufRef;
        std::size_t begin;
        std::size_t end;
    };

    /**
     * @brief 尝试 [begin, end) 字节范围的内容, 如果没有, 则会请求.
     * 如果 它不属于同一个块, 则仅返回前面的块
     */
    template <typename Func>
    RangeRequestCache tryGetCache(std::size_t begin, std::size_t end, Func const& cb) {
        const auto endIdx = (end - 1) / BlockMaxSize;
        for (std::size_t i = begin / BlockMaxSize; i <= endIdx; ++i) {
            if (!_blockArr[i]) {
                rangeRequest(i, cb);
            }
            return {
                *_blockArr[i],
                i * BlockMaxSize,
                std::min((i + 1) * BlockMaxSize, _maxLen)
            };
        }
        throw std::runtime_error {"tryGetCache: Index out of bounds"};
    }

    bool isEffective(std::size_t begin, std::size_t end) const {
        const auto endIdx = (end - 1) / BlockMaxSize;
        for (std::size_t i = begin / BlockMaxSize; i <= endIdx; ++i)
            if (!_blockArr[i])
                return false;
        return true;
    }

private:
    template <typename Func>
    auto rangeRequest(std::size_t idx, Func const& cb) -> decltype(
        std::is_same_v<QByteArray, decltype(std::declval<Func>()(idx, idx))>, void()
    ) {
        std::size_t l = idx * BlockMaxSize;
        std::size_t r = std::min(_maxLen, l + BlockMaxSize);
        _blockArr[idx] = cb(l, r);
    }

    // 最大长度
    std::size_t _maxLen;

    // 分块数组
    std::vector<std::optional<QByteArray>> _blockArr;
};

} // namespace HX

#endif // !_HX_RANGE_REQUEST_CACHE_MANAGEMENT_H_