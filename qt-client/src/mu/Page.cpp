#include <mu/Page.h>

#include <mupdf/fitz.h>

#include <mu/Document.h>

static inline void _clearRgbSamplesWithValue(unsigned char* samples, int size,
                                             int b, int g, int r, int a) {
    int i = 0;

    while (i < size) {
        *(samples + i++) = r;
        *(samples + i++) = g;
        *(samples + i++) = b;
        *(samples + i++) = a;
    }
}

static inline void _imageCleanupHandler(void* data) {
    unsigned char* samples = static_cast<unsigned char*>(data);

    if (samples) [[unlikely]] {
        delete[] samples;
    }
}

namespace HX::Mu {

Page::Page(Document const& doc, int index) noexcept
    : _doc(doc)
    , _textItems()
{
    fz_try(_doc._ctx) {
        fz_device* listDevice{}; // 渲染设备

        // 加载页面
        _page = fz_load_page(_doc._ctx, _doc._doc, index);

        // 获取页面的边界框
        auto rect = fz_bound_page(_doc._ctx, _page);

        // 创建显示列表
        _displayList = fz_new_display_list(_doc._ctx, rect);

        // 创建用于预渲染的 List Device, 并绑定到显示列表
        listDevice = fz_new_list_device(_doc._ctx, _displayList);

        // 解析页面内容(文本、图像等), 并写入 _displayList
        fz_run_page_contents(_doc._ctx, _page, listDevice, fz_identity, nullptr);
        
        fz_close_device(_doc._ctx, listDevice);
        fz_drop_device(_doc._ctx, listDevice);
    } fz_catch(_doc._ctx) [[unlikely]] {
        // 一般不会进入这里
    }
}

QSizeF Page::size() const {
    auto rect = fz_bound_page(_doc._ctx, _page);
    return {rect.x1 - rect.x0, rect.y1 - rect.y0};
}

QImage Page::renderImage(float dpi, float rotation) const {
    float scale = dpi / 72.f;
    fz_pixmap* pixmap = nullptr;
    unsigned char* samples = nullptr;
    unsigned char* copyed_samples = nullptr;
    int width = 0;
    int height = 0;
    int size = 0;

    fz_stext_page* text_page =
        fz_new_stext_page(_doc._ctx, fz_bound_page(_doc._ctx, _page));

    fz_device* tdev;
    tdev = fz_new_stext_device(_doc._ctx, text_page, nullptr);
    fz_run_display_list(_doc._ctx, _displayList, tdev, fz_identity,
                        fz_infinite_rect, nullptr);
    fz_close_device(_doc._ctx, tdev);
    fz_drop_device(_doc._ctx, tdev);

    // 构建缩放矩阵
    fz_matrix transform = fz_pre_rotate(fz_scale(scale, scale), rotation);

    // 转换画面大小
    fz_irect bbox = fz_round_rect(
        fz_transform_rect(
            fz_bound_page(_doc._ctx, _page), transform));
    fz_rect bounds = fz_rect_from_irect(bbox);

    // 渲染到位图
    fz_device* dev = nullptr;
    fz_try(_doc._ctx) {
        // 创建 fz_pixmap, 并且其包含透明通道
        pixmap = fz_new_pixmap_with_bbox(_doc._ctx, fz_device_rgb(_doc._ctx),
                                         bbox, nullptr, 1);

        if (!_doc._transparent) {
            if (_doc._b == 255 && _doc._g == 255 && _doc._r == 255 && _doc._a == 255) {
                // 白色背景
                fz_clear_pixmap_with_value(_doc._ctx, pixmap, 0xFF);
            } else {
                // 自定义背景
                _clearRgbSamplesWithValue(samples, size, _doc._b, _doc._g,
                    _doc._r, _doc._a);
            }
        }
        dev = fz_new_draw_device(_doc._ctx, transform, pixmap);
        fz_run_display_list(_doc._ctx, _displayList, dev, transform, bounds,
                            nullptr);

        samples = fz_pixmap_samples(_doc._ctx, pixmap);
        width = fz_pixmap_width(_doc._ctx, pixmap);
        height = fz_pixmap_height(_doc._ctx, pixmap);
        size = width * height * 4;
    } fz_always(_doc._ctx) {
        if (dev) {
            fz_close_device(_doc._ctx, dev);
            fz_drop_device(_doc._ctx, dev);
        }
        dev = nullptr;
    } fz_catch(_doc._ctx) [[unlikely]] {
        if (pixmap) {
            fz_drop_pixmap(_doc._ctx, pixmap);
        }
        pixmap = nullptr;
    }

    // 渲染到 QImage
    QImage image{};
    if (!pixmap) [[unlikely]] {
        return image;
    }
    copyed_samples = new unsigned char[size];
    memcpy(copyed_samples, samples, size);
    fz_drop_pixmap(_doc._ctx, pixmap);

    image = QImage{copyed_samples, width, height, QImage::Format_RGBA8888,
                   _imageCleanupHandler, copyed_samples};
    return image;
}

std::vector<TextItem> Page::renderText(float scale, float rotation) const {
    std::vector<TextItem> textItems;
    fz_stext_page* textPage = fz_new_stext_page_from_page(_doc._ctx, _page, nullptr);
    if (!textPage) [[unlikely]] {
        return textItems;
    }

    // 创建缩放和旋转矩阵
    fz_matrix scale_matrix = fz_scale(scale, scale);
    fz_matrix transform = fz_pre_rotate(scale_matrix, rotation);
    fz_matrix rotate_matrix = fz_rotate(rotation);
    fz_matrix ctm = fz_concat(scale_matrix, rotate_matrix); // 组合变换矩阵

    // 获取变换后的页面边界框
    fz_rect pageBounds = fz_transform_rect(fz_bound_page(_doc._ctx, _page), transform);

    // debug 的
    textItems.push_back({
        .text = {},
        .rect = {
            pageBounds.x0, pageBounds.y0,
            pageBounds.x1 - pageBounds.x0,  // 宽度
            pageBounds.y1 - pageBounds.y0   // 高度
        },
        .color = {255, 0, 0, 255}  // 使用红色高对比度框出页面
    });


    for (fz_stext_block* block = textPage->first_block; block; block = block->next) {
        if (block->type != FZ_STEXT_BLOCK_TEXT) 
            continue;

        for (fz_stext_line* line = block->u.t.first_line; line; line = line->next) {
            for (fz_stext_char* ch = line->first_char; ch; ch = ch->next) {
                // 文字、颜色
                QString charText(QChar(ch->c));
                QColor charColor(
                    (ch->argb >> 16) & 0xFF,
                    (ch->argb >> 8) & 0xFF,
                    ch->argb & 0xFF,
                    (ch->argb >> 24) & 0xFF
                );

                // 字体
                fz_font* font = ch->font;
                const char* font_family = "sans-serif";
                if (fz_font_is_monospaced(_doc._ctx, font))
                    font_family = "monospace";
                else if (fz_font_is_serif(_doc._ctx, font))
                    font_family = "serif";
                const char* fontName = font ? fz_font_name(_doc._ctx, font) : "Default";
                QFont charFont(
                    QString::fromUtf8(fontName)
                );
                charFont.setFamily(font_family);
                charFont.setWeight(fz_font_is_bold(_doc._ctx, font)
                    ? QFont::Bold
                    : QFont::Normal
                );
                charFont.setStyle(fz_font_is_italic(_doc._ctx, font) 
                    ? QFont::Style::StyleItalic
                    : QFont::Style::StyleNormal
                );

                // 警告! 这个只能是 setPixelSize 而不能是 setPointSize
                charFont.setPixelSize(ch->size * scale);

                // 修正坐标计算: 使用 fz_transform_quad()
                fz_quad quadTransformed = fz_transform_quad(ch->quad, ctm);
                QRectF charRect(
                    quadTransformed.ul.x, quadTransformed.ul.y,
                    quadTransformed.lr.x - quadTransformed.ul.x,
                    quadTransformed.lr.y - quadTransformed.ul.y
                );

                // 变换字符的原点
                QPointF charOrigin(ch->origin.x * scale, ch->origin.y * scale);

                textItems.emplace_back(TextItem{
                    charText,
                    charRect,
                    charFont,
                    charColor,
                    charOrigin
                });
            }
        }
    }

    fz_drop_stext_page(_doc._ctx, textPage);
    return textItems;
}

std::vector<TextItem> Page::renderTextLine(float scale, float rotation) const {
    std::vector<TextItem> textItems;
    fz_stext_page* text_page = fz_new_stext_page_from_page(_doc._ctx, _page, nullptr);
    if (!text_page) [[unlikely]] {
        return textItems;
    }

    // 构建变换矩阵
    fz_matrix transform = fz_pre_rotate(fz_scale(scale, scale), rotation);
    
    // 创建缩放和旋转矩阵
    fz_matrix scale_matrix = fz_scale(scale, scale);
    fz_matrix rotate_matrix = fz_rotate(rotation);
    fz_matrix ctm = fz_concat(scale_matrix, rotate_matrix); // 组合变换矩阵

    // 获取变换后的页面边界框
    fz_rect pageBounds = fz_transform_rect(fz_bound_page(_doc._ctx, _page), transform);

    textItems.push_back({
        .text = {},
        .rect = {
            pageBounds.x0, pageBounds.y0,
            pageBounds.x1 - pageBounds.x0,  // 宽度
            pageBounds.y1 - pageBounds.y0   // 高度
        },
        .color = {255, 0, 0, 255}  // 使用红色高对比度框出页面
    });

    QString currentLineText;
    QRectF currentLineRect;
    QFont currentLineFont;
    QColor currentLineColor;
    QPointF currentLineOrigin;
    bool isNewLine = true;

    fz_stext_char* prevChar = nullptr;

    for (fz_stext_block* block = text_page->first_block; block; block = block->next) {
        if (block->type != FZ_STEXT_BLOCK_TEXT) 
            continue;

        for (fz_stext_line* line = block->u.t.first_line; line; line = line->next) {
            for (fz_stext_char* ch = line->first_char; ch; ch = ch->next) {
                // 文字、颜色
                QString charText(QChar(ch->c));
                QColor charColor(
                    (ch->argb >> 16) & 0xFF,
                    (ch->argb >> 8) & 0xFF,
                    ch->argb & 0xFF,
                    (ch->argb >> 24) & 0xFF
                );

                // 字体
                fz_font* font = ch->font;
                const char* font_family = "sans-serif";
                if (fz_font_is_monospaced(_doc._ctx, font))
                    font_family = "monospace";
                else if (fz_font_is_serif(_doc._ctx, font))
                    font_family = "serif";
                const char* fontName = font ? fz_font_name(_doc._ctx, font) : "Default";
                QFont charFont(
                    QString::fromUtf8(fontName)
                );
                charFont.setFamily(font_family);
                charFont.setWeight(fz_font_is_bold(_doc._ctx, font)
                    ? QFont::Bold
                    : QFont::Normal
                );
                charFont.setStyle(fz_font_is_italic(_doc._ctx, font) 
                    ? QFont::Style::StyleItalic
                    : QFont::Style::StyleNormal
                );

                // 警告! 这个只能是 setPixelSize 而不能是 setPointSize
                charFont.setPixelSize(ch->size * scale);

                // 修正坐标计算: 使用 fz_transform_quad()
                fz_quad quadTransformed = fz_transform_quad(ch->quad, ctm);
                QRectF charRect(
                    quadTransformed.ul.x, quadTransformed.ul.y,
                    quadTransformed.lr.x - quadTransformed.ul.x,
                    quadTransformed.lr.y - quadTransformed.ul.y
                );

                // 变换字符的原点
                QPointF charOrigin(ch->origin.x * scale, ch->origin.y * scale);

                // 检测是否需要新建一行
                if (isNewLine 
                    || !prevChar 
                    || (fabs(prevChar->origin.y - ch->origin.y) > 1e-3f) // 不是同一行
                    || (prevChar->size != ch->size) // 字号不同
                    || (prevChar->font != ch->font) // 字体不同
                    || (prevChar->argb != ch->argb)
                ) { // 颜色不同

                    // 存储上一行
                    if (!currentLineText.isEmpty()) {
                        textItems.emplace_back(TextItem{
                            currentLineText,
                            currentLineRect,
                            currentLineFont,
                            currentLineColor,
                            currentLineOrigin
                        });
                    }

                    // 开始新行
                    currentLineText = charText;
                    currentLineRect = charRect;
                    currentLineFont = charFont;
                    currentLineColor = charColor;
                    currentLineOrigin = charOrigin;
                    isNewLine = false;
                } else {
                    // 继续合并同一行
                    currentLineText += charText;
                    currentLineRect.setRight(charRect.right());
                    currentLineRect.setBottom(
                        qMax(currentLineRect.bottom(), charRect.bottom())
                    );
                }

                prevChar = ch;
            }
        }
    }

    // 存储最后一行
    if (!currentLineText.isEmpty()) {
        textItems.emplace_back(TextItem{
            currentLineText,
            currentLineRect,
            currentLineFont,
            currentLineColor,
            currentLineOrigin
        });
    }

    fz_drop_stext_page(_doc._ctx, text_page);
    return textItems;
}


Page::~Page() noexcept {
    if (_displayList) [[likely]] {
        fz_drop_display_list(_doc._ctx, _displayList);
    }
    if (_page) [[likely]] {
        fz_drop_page(_doc._ctx, _page);
    }
}

} // namespace HX::Mu