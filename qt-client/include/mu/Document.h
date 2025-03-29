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
#ifndef _HX_MU_DOCUMENT_H_
#define _HX_MU_DOCUMENT_H_

#include <mu/StreamFuncBuilder.hpp>

struct fz_context;
struct fz_stream;
struct fz_document;

namespace HX::Mu {

/**
 * @brief MuPdf库的 基本文档解析类的封装
 */
class Document {
public:
    explicit Document(const char* filePath) noexcept;

    /**
     * @brief 自定义输入流回调函数, 子类可以重写这个方法, 以自定义输入流回调函数 (基类默认什么都不做, 使用默认的流)
     * @param builder 建造者
     * @return Document& 
     */
    virtual Document& setStream(StreamFuncBuilder const& builder);

    /**
     * @brief 生成文档, 如果有自定义流, 则优先基于自定义流生成
     * @param filePath 文件路径, 特别是使用默认流的时候
     * @throw std::runtime_error
     */
    void buildDocument(const char* filePath);

    /**
     * @brief 获取文档的总页数
     * @throw std::runtime_error
     * @return int 总页码数
     */
    int pageCount() const;

    virtual ~Document() noexcept;

private:
    // 禁止拷贝赋值和拷贝构造
    Document(Document const&) = delete;
    Document& operator=(Document const&) = delete;

protected:
    fz_context* _ctx;
    fz_stream* _stream;
    fz_document* _doc;
};

} // namespace HX::Mu

#endif // !_HX_MU_DOCUMENT_H_