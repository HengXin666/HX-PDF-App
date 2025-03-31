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

#include <map>
#include <memory>

#include <QString>
#include <QDateTime>

#include <mu/StreamFuncBuilder.hpp>

struct fz_context;
struct fz_stream;
struct fz_document;

namespace HX::Mu {

class Page;

/**
 * @brief MuPdf库的 基本文档解析类的封装
 */
class Document {
    friend Page;
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

    /**
     * @brief 获取第 index 页
     * @param index 页码
     * @throw std::runtime_error 获取失败/页码越界/文档未加载
     * @return std::shared_ptr<Page> 结果
     */
    std::shared_ptr<Page> page(int index);

    /**
     * @brief 判断文档是否需要密码
     * @throw std::runtime_error 需要保证文件已经加载 (调用了`buildDocument`方法)
     * @return true 需要密码
     * @return false 不需要密码
     */
    bool needsPassword() const;

    /**
     * @brief 验证密码
     * @param password 密码
     * @throw std::runtime_error 需要保证文件已经加载 (调用了`buildDocument`方法)
     * @return true 密码正确
     * @return false 密码错误
     */
    bool authPassword(const QString& password);

    // 下面函数只支持对pdf进行解析!!!

    QString title() const;
    QString author() const;
    QString subject() const;
    QString keywords() const;
    QString creator() const;
    QString producer() const;
    QDateTime creationDate() const;
    QDateTime modDate() const;

    /**
     * @brief 设置背景是否透明 (默认不透明 = false)
     * @param transparent 
     */
    void setTransparent(bool transparent) {
        _transparent = transparent;
    }

    /**
     * @brief 设置背景颜色
     * @param r 
     * @param g 
     * @param b 
     * @param a 透明通道 (alpha) (默认不透明 = 255)
     */
    void setBackgroundColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) {
        _r = r;
        _g = g;
        _b = b;
        _a = a;
    }

    virtual ~Document() noexcept;
private:
    // 禁止拷贝赋值和拷贝构造
    Document(Document const&) = delete;
    Document& operator=(Document const&) = delete;

    // 仅支持对pdf进行解析!!!
    QString info(const char* key) const;

    // 维护页面: 页码 - 页码渲染体
    // 因为 RAII, 因此不会内存泄漏, Page的析构会自行管理
    // 生命周期: Document > Page
    std::map<int, std::shared_ptr<Page>> _pageList;

    // 总页码数
    mutable int _pageCnt = -1;

    // 渲染的背景是否透明 (默认不透明)
    int _transparent = false;

    // 背景颜色 RGBA
    int _r = 255, _g = 255, _b = 255, _a = 255;
protected:
    fz_context* _ctx;
    fz_stream* _stream;
    fz_document* _doc;
};

} // namespace HX::Mu

#endif // !_HX_MU_DOCUMENT_H_