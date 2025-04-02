#include <mu/Page.h>

#include <mupdf/fitz.h>

#include <mu/Document.h>

static void clear_rgb_samples_with_value(unsigned char* samples, int size,
                                         int b, int g, int r, int a) {
    int i = 0;

    while (i < size) {
        *(samples + i++) = r;
        *(samples + i++) = g;
        *(samples + i++) = b;
        *(samples + i++) = a;
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

        int dpi = 96; // 设置 dpi
        fz_matrix ctm = fz_scale(dpi / 72.0f, dpi / 72.0f);

        // 解析页面内容(文本、图像等), 并写入 _displayList
        fz_run_page_contents(_doc._ctx, _page, listDevice, ctm, nullptr);
        
        fz_close_device(_doc._ctx, listDevice);
        fz_drop_device(_doc._ctx, listDevice);
    } fz_catch(_doc._ctx) {
        // [[unlikely]] 一般不会进入这里
    }
}

QSizeF Page::size() const {
    auto rect = fz_bound_page(_doc._ctx, _page);
    return {rect.x1 - rect.x0, rect.y1 - rect.y0};
}

QImage Page::renderImage(float dpiX, float dpiY, float rotation) const {
    float scaleX = dpiX / 72.0f;
    float scaleY = dpiY / 72.0f;
    
    fz_pixmap* pixmap = nullptr;
    unsigned char* samples = nullptr;
    unsigned char* copyedSamples = nullptr;
    int width = 0;
    int height = 0;
    int size = 0;

    // 创建一个空的页面
    fz_stext_page* textPage =
        fz_new_stext_page(_doc._ctx, fz_bound_page(_doc._ctx, _page));
    
    fz_device* tdev = nullptr;
    fz_try(_doc._ctx) {
        // 创建一个文本设备 (用于接收渲染的文本)
        tdev = fz_new_stext_device(_doc._ctx, textPage, nullptr);

        // 使用文本设备渲染显示列表, 将文本提取到 textPage
        fz_run_display_list(_doc._ctx, _displayList, tdev, fz_identity,
                            fz_infinite_rect, nullptr);
        fz_close_device(_doc._ctx, tdev);
        fz_drop_device(_doc._ctx, tdev);
    } fz_catch(_doc._ctx) [[unlikely]] {
        if (tdev) {
            fz_close_device(_doc._ctx, tdev);
            fz_drop_device(_doc._ctx, tdev);
            tdev = nullptr;
        }
        fz_drop_stext_page(_doc._ctx, textPage);
        return QImage();
    }

    // 构建变换矩阵
    fz_matrix transform = fz_pre_rotate(fz_scale(scaleX, scaleY), rotation);
    
    // 获得转换页面的大小
    fz_rect bounds = fz_bound_page(_doc._ctx, _page);
    fz_irect bbox = fz_round_rect(fz_transform_rect(bounds, transform));
    bounds = fz_rect_from_irect(bbox);

    // 渲染到 fz_pixmap
    fz_device* dev = nullptr;
    fz_try(_doc._ctx) {
        pixmap = fz_new_pixmap_with_bbox(_doc._ctx, fz_device_rgb(_doc._ctx),
                                         bbox, nullptr, 1);

        if (!_doc._transparent) {
            if (_doc._b == 255 && _doc._g == 255 && _doc._r == 255 && _doc._a == 255) {
                fz_clear_pixmap_with_value(_doc._ctx, pixmap, 0xFF);
            } else {
                clear_rgb_samples_with_value(samples, size, _doc._b, _doc._g,
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
            dev = nullptr;
        }
    } fz_catch(_doc._ctx) [[unlikely]] {
        if (pixmap) {
            fz_drop_pixmap(_doc._ctx, pixmap);
            pixmap = nullptr;
        }
        fz_drop_stext_page(_doc._ctx, textPage);
        return QImage{};
    }
    
    // 渲染到 QImage
    QImage image;
    if (!pixmap) [[unlikely]] {
        fz_drop_stext_page(_doc._ctx, textPage);
        return image;
    }
    
    copyedSamples = new unsigned char[size];
    memcpy(copyedSamples, samples, size);
    fz_drop_pixmap(_doc._ctx, pixmap);

    image = QImage(
        copyedSamples, width, height, QImage::Format_RGBA8888,
        [](void* data) {
            if (data) [[likely]] {
                delete[] static_cast<unsigned char*>(data); 
            }
        },
        copyedSamples
    );

    fz_drop_stext_page(_doc._ctx, textPage);
    return image;
}

QImage Page::renderOnlyDraw(float scaleX, float scaleY, float rotation) const {
    fz_pixmap* pixmap = nullptr;
    unsigned char* samples = nullptr;
    unsigned char* copyedSamples = nullptr;
    int width = 0;
    int height = 0;
    int size = 0;

    // 创建一个空的页面
    fz_stext_page* textPage =
        fz_new_stext_page(_doc._ctx, fz_bound_page(_doc._ctx, _page));
    
    fz_try(_doc._ctx) {
        // 创建一个文本设备 (用于接收渲染的文本)
        fz_device* tdev = fz_new_stext_device(_doc._ctx, textPage, nullptr);

        // 使用文本设备渲染显示列表, 将文本提取到 textPage
        fz_run_display_list(_doc._ctx, _displayList, tdev, fz_identity,
                            fz_infinite_rect, nullptr);
        fz_close_device(_doc._ctx, tdev);
        fz_drop_device(_doc._ctx, tdev);
    } fz_catch(_doc._ctx) [[unlikely]] {
        fz_drop_stext_page(_doc._ctx, textPage);
        return QImage();
    }

    // 构建变换矩阵
    fz_matrix transform = fz_pre_rotate(fz_scale(scaleX, scaleY), rotation);
    
    // 获得转换页面的大小
    fz_irect bbox = fz_round_rect(
        fz_transform_rect(fz_bound_page(_doc._ctx, _page), transform));
    fz_rect bounds = fz_rect_from_irect(bbox);

    // 渲染到 fz_pixmap
    fz_device* dev = nullptr;
    fz_try(_doc._ctx) {
        pixmap = fz_new_pixmap_with_bbox(_doc._ctx, fz_device_rgb(_doc._ctx),
                                         bbox, nullptr, 1);

        if (!_doc._transparent) {
            samples = fz_pixmap_samples(_doc._ctx, pixmap);
            if (!samples) [[unlikely]] {
                fz_throw(_doc._ctx, FZ_ERROR_GENERIC,
                         "Failed to get pixmap samples");
            }

            if (_doc._b == 255 && _doc._g == 255 && _doc._r == 255 && _doc._a == 255) {
                fz_clear_pixmap_with_value(_doc._ctx, pixmap, 0xFF);
            } else {
                clear_rgb_samples_with_value(samples, size, _doc._b, _doc._g,
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
    } fz_catch(_doc._ctx) [[unlikely]] {
        if (pixmap) {
            fz_drop_pixmap(_doc._ctx, pixmap);
        }
        fz_drop_stext_page(_doc._ctx, textPage);
        return QImage();
    }
    
    // 渲染到 QImage
    QImage image;
    if (!pixmap) [[unlikely]] {
        fz_drop_stext_page(_doc._ctx, textPage);
        return image;
    }
    
    copyedSamples = new unsigned char[size];
    memcpy(copyedSamples, samples, size);
    fz_drop_pixmap(_doc._ctx, pixmap);

    image = QImage(
        copyedSamples, width, height, QImage::Format_RGBA8888,
        [](void* data) {
            if (data) [[likely]] {
                delete[] static_cast<unsigned char*>(data); 
            }
        },
        copyedSamples
    );

    fz_drop_stext_page(_doc._ctx, textPage);
    return image;
}

#if 0

// Draws current page into texture using pixmap
static int texture_width, texture_height;
bool redrawBuffer() {
#ifdef DEBUG
    printf("MUDocument::redrawBuffer pp\n");
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();
#endif
    // fz_scale(&m_transform, m_scale / 72, m_scale / 72);
    // fz_pre_rotate(&m_transform, m_rotate);

    fz_drop_stext_page(m_ctx, m_pageText);
    m_pageText = nullptr;
    fz_drop_link(m_ctx, m_links);
    m_links = nullptr;
    fz_drop_page(m_ctx, m_page);
    m_page = nullptr;

    m_page = fz_load_page(m_ctx, m_doc, m_current_page);
    m_links = fz_load_links(m_ctx, m_page);
    m_pageText = fz_new_stext_page_from_page(m_ctx, m_page, nullptr);

#ifdef DEBUG
    printf("fz_load\n");
#endif

    // bounds for inital window size
    m_bounds = fz_bound_page(m_ctx, m_page);
#ifdef DEBUG
    printf("bound_page; (%f, %f) - (%f, %f)\n", m_bounds.x0, m_bounds.y0,
           m_bounds.x1, m_bounds.y1);
#endif

    fz_matrix rotation_matrix;
    fz_matrix scaling_matrix;
    fz_matrix translation_matrix;

    // Rotate first since co-ords can be negative
    rotation_matrix = fz_rotate(m_rotate); // m_t = rotate * scaling_matrix
    fz_transform_rect(m_bounds, rotation_matrix);

    // Translate to positive coords to figure out fit to width/height scale
    // easily
    translation_matrix = fz_translate(
        -m_bounds.x0, -m_bounds.y0); // matrix = translation matrix (for rect)
    fz_transform_rect(m_bounds, translation_matrix);

    if (m_fitWidth) {
        m_scale = m_width / (m_bounds.x1 - m_bounds.x0);
        vector<float> vec(std::begin(zoomLevels), std::end(zoomLevels));
        auto const it = std::lower_bound(vec.begin(), vec.end(), m_scale);
        if (it != vec.end())
            zoomLevel = it - vec.begin();
    } else if (m_fitHeight) {
        m_scale = m_height / (m_bounds.y1 - m_bounds.y0);
        vector<float> vec(std::begin(zoomLevels), std::end(zoomLevels));
        auto const it = std::lower_bound(vec.begin(), vec.end(), m_scale);
        if (it != vec.end())
            zoomLevel = it - vec.begin();
    }

#ifdef DEBUG
    printf("bound_page; m_scale: %2.3gx, zoomLevel: %i\n", m_scale, zoomLevel);
#endif

    // Scaling is then always positive so do it last
    scaling_matrix = fz_scale(m_scale, m_scale);
    fz_transform_rect(m_bounds, scaling_matrix);

    // Create final transformation matrix in the correct order (Rotation x
    // Scaling x Translation)
    m_transform = fz_concat(rotation_matrix, scaling_matrix);
    // fz_concat(&m_transform, &m_transform, &translation_matrix); // dont
    // really need since you transfromed it with translation already

#ifdef DEBUG
    printf("scale/transform\n");
#endif

    // TODO: Is display list or bbox better?
    // This is currently the longest operation
    pdf_annot* annot;
    fz_try(m_ctx) m_pix = fz_new_pixmap_from_page_contents(
        m_ctx, m_page, m_transform, fz_device_rgb(m_ctx), 0);
    fz_catch(m_ctx) {
        printf("cannot render page: %s\n", fz_caught_message(m_ctx));
    }

#ifdef DEBUG
    printf("new_pixmap n: %i \n", m_pix->n);
#endif

    texture_width = m_pix->w;
    texture_height = m_pix->h;

#ifdef __vita__
    // Crashes due to GPU memory use without this.
    vita2d_free_texture(texture);

#ifdef DEBUG
    printf("post vita2d_free_texture\n");
#endif

    texture = _vita2d_load_pixmap_generic(m_pix);
#else
    if (texture == nullptr)
        texture = std::make_unique<Texture2D>();
    texture->Generate(m_pix->w, m_pix->h, m_pix->samples);
#endif

#ifdef DEBUG
    printf("post _vita2d_load_pixmap_generic m_pix->n: %i\n", m_pix->n);
#endif

    // fz_drop_pixmap(m_ctx, m_pix);
    // load annotations

    return true;
}

#endif // !0

std::vector<TextItem> Page::testGetText(float scale, float rotation) const {
    std::vector<TextItem> textItems;
    fz_stext_page* text_page = fz_new_stext_page_from_page(_doc._ctx, _page, nullptr);
    if (!text_page) [[unlikely]] {
        return textItems;
    }

    // 创建缩放和旋转矩阵
    fz_matrix scale_matrix = fz_scale(scale, scale);
    fz_matrix rotate_matrix = fz_rotate(rotation);
    fz_matrix ctm = fz_concat(scale_matrix, rotate_matrix); // 组合变换矩阵

    QString currentLineText;
    QRectF currentLineRect;
    QFont currentLineFont;
    QColor currentLineColor;
    QPointF currentLineOrigin;
    bool isNewLine = true;

    fz_stext_char* prevChar = nullptr;

    for (fz_stext_block* text_block = text_page->first_block; text_block; text_block = text_block->next) {
        if (text_block->type != FZ_STEXT_BLOCK_TEXT) 
            continue;

        for (fz_stext_line* text_line = text_block->u.t.first_line; text_line; text_line = text_line->next) {
            for (fz_stext_char* ch = text_line->first_char; ch; ch = ch->next) {
                // 文字、颜色
                QString charText(QChar(ch->c));
                QColor charColor(
                    (ch->argb >> 16) & 0xFF,
                    (ch->argb >> 8) & 0xFF,
                    ch->argb & 0xFF,
                    (ch->argb >> 24) & 0xFF
                );

                // 字体
                const char* fontName = ch->font ? fz_font_name(_doc._ctx, ch->font) : "Default";
                QFont charFont(
                    QString::fromUtf8(fontName), 
                    static_cast<int>(ch->size * scale)
                );

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
                if (isNewLine || !prevChar ||
                    (fabs(prevChar->origin.y - ch->origin.y) > 1e-2) || // 不是同一行
                    (prevChar->size != ch->size) || // 字号不同
                    (prevChar->font != ch->font) || // 字体不同
                    (prevChar->argb != ch->argb)) { // 颜色不同

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
                    currentLineRect.setBottom(qMax(currentLineRect.bottom(), charRect.bottom()));
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


std::vector<TextItem> Page::testGetTextLine(float scale, float rotation) const {
    std::vector<TextItem> textItems;
    fz_stext_page* text_page = fz_new_stext_page_from_page(_doc._ctx, _page, nullptr);
    if (!text_page) [[unlikely]] {
        return textItems;
    }

    // 创建缩放和旋转矩阵
    fz_matrix scale_matrix = fz_scale(scale, scale);
    fz_matrix rotate_matrix = fz_rotate(rotation);
    fz_matrix ctm = fz_concat(scale_matrix, rotate_matrix); // 组合变换矩阵

    for (fz_stext_block* block = text_page->first_block; block; block = block->next) {
        if (block->type != FZ_STEXT_BLOCK_TEXT) 
            continue;

        for (fz_stext_line* line = block->u.t.first_line; line; line = line->next) {
            QString lineText;
            QRectF lineRect;
            QFont lineFont;
            QColor lineColor;
            QPointF lineOrigin;

            lineRect.setRect(
                line->bbox.x0 * scale, 
                line->bbox.y0 * scale, 
                (line->bbox.x1 - line->bbox.x0) * scale, 
                (line->bbox.y1 - line->bbox.y0) * scale
            );

            if (line->first_char) {
                fz_font* font = line->first_char->font;
                // const char* font_family = "sans-serif";
                // const char* font_weight = "normal";
                // const char* font_style = "normal";
                // if (fz_font_is_monospaced(_doc._ctx, font))
                //     font_family = "monospace";
                // else if (fz_font_is_serif(_doc._ctx, font))
                //     font_family = "serif";
                // if (fz_font_is_bold(_doc._ctx, font))
                //     font_weight = "bold";
                // if (fz_font_is_italic(_doc._ctx, font))
                //     font_style = "italic";
                lineFont.setStyleName(fz_font_name(_doc._ctx, font));
                lineFont.setPointSizeF(line->first_char->size * scale);
                lineOrigin.setX(line->first_char->origin.x * scale);
                lineOrigin.setY(line->first_char->origin.y * scale);
                lineColor = {
                    static_cast<int>((line->first_char->argb >> 16) & 0xFF),
                    static_cast<int>((line->first_char->argb >>  8) & 0xFF),
                    static_cast<int>((line->first_char->argb >>  0) & 0xFF),
                    static_cast<int>((line->first_char->argb >> 24) & 0xFF)
                };
            }

            for (fz_stext_char* ch = line->first_char; ch; ch = ch->next) {
                lineText += QChar(ch->c);
            }
            
            textItems.emplace_back(TextItem{
                lineText,
                lineRect,
                lineFont,
                lineColor,
                lineOrigin
            });
        }
    }

    fz_drop_stext_page(_doc._ctx, text_page);
    return textItems;
}

std::vector<TextItem> Page::testGetTextTest(float scale, float rotation) const {
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

    for (fz_stext_block* text_block = text_page->first_block; text_block; text_block = text_block->next) {
        if (text_block->type != FZ_STEXT_BLOCK_TEXT) 
            continue;

        for (fz_stext_line* text_line = text_block->u.t.first_line; text_line; text_line = text_line->next) {
            for (fz_stext_char* ch = text_line->first_char; ch; ch = ch->next) {
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