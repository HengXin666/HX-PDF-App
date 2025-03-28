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
#ifndef _HX_STREAM_FUNC_BUILDER_H_
#define _HX_STREAM_FUNC_BUILDER_H_

#include <cstdint>

struct fz_context;
struct fz_stream;

namespace HX::Mu {

/**
 * @brief MuPdf库的自定义流所需的三个回调函数的建造者
 */
struct StreamFuncBuilder {
    /**
     * @brief 读取数据
     * @param ctx 
     * @param stm [in, out] [rp, wp) 之间的数据是有效的, 如果有数据, 会更新 [rp, wq) 指向新的数据, 并且返回 *stm->rp++
     * @param max 期望读取的最大字节数
     * @return int 没有数据则返回-1, 否则返回读取到的字节数
     */
    using NextFuncPtr = int (*)(fz_context* ctx, fz_stream* stm, size_t max);

    /**
     * @brief 清理流的内部状态
     * @param ctx 
     * @param state 
     */
    using DropFuncPtr = void (*)(fz_context* ctx, void* state);

    /**
     * @brief 随机读取
     * @param ctx 
     * @param stm 
     * @param offset 偏移量
     * @param whence 基于何位置偏移: 文件头/文件尾/当前文件指针
     */
    using SeekFuncPtr = void (*)(fz_context* ctx, fz_stream* stm, int64_t offset, int whence);

    StreamFuncBuilder() = default;

    StreamFuncBuilder(
        NextFuncPtr next,
        DropFuncPtr drop,
        SeekFuncPtr seek
    )
        : _next(next)
        , _drop(drop)
        , _seek(seek)
    {}

    NextFuncPtr _next;
    DropFuncPtr _drop;
    SeekFuncPtr _seek;
};

} // namespace HX::Mu

#endif // !_HX_STREAM_FUNC_BUILDER_H_