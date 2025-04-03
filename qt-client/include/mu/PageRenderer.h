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
#ifndef _HX_PAGE_RENDERER_H_
#define _HX_PAGE_RENDERER_H_

#include <QThread>

namespace HX::Mu {

/*  架构

    单一原则的文档类, 仅维护自己的数据
    Document {ctx, doc, stream} -> Page {0...n-1} -> 渲染出 QImg

    渲染器, 内部维护有渲染队列和页面缓存
    // 叫 Document 给我 Page, 然后渲染 (in 子线程) -> 通知ui线程
    PageRenderer : QT线程类 {
    槽:
        渲染(第 i 页) -> 提交任务到子线程
        加载新文档(Document*)

    信号:
        渲染完成(第 i 页, 数据)

    data:
         Document* 
    }

    主画面渲染控件
    QMuWidget {
    槽:
        下一页()
        上一页()
        跳转到(第 i 页)
        设置页面大小(新大小)
        加载文档(url)
    
    事件:
        鼠标点击 -> 判断是否处于 url 矩形内, 是则打开链接

    data:
        PageRenderer
        LRUCache<int, QImg>
    }
*/

class PageRenderer : public QThread {
    Q_OBJECT
public:
    explicit PageRenderer(QObject* parent = nullptr);
};

} // namespace HX::Mu

#endif // !_HX_PAGE_RENDERER_H_