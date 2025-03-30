#include <mu/Page.h>

#include <mupdf/fitz.h>

#include <mu/Document.h>

static void clear_rgb_samples_with_value(
    unsigned char *samples, int size,
    int b, int g, int r, int a)
{
int i = 0;

while (i < size)
{
    *(samples + i++) = r;
    *(samples + i++) = g;
    *(samples + i++) = b;
    *(samples + i++) = a;
}
}

/**
 * @brief Clean up image data when the last copy of the QImage is destoryed.
 */
static inline void imageCleanupHandler(void* data) {
    unsigned char* samples = static_cast<unsigned char*>(data);

    if (samples) {
        delete[] samples;
    }
}

namespace HX::Mu {

Page::Page(Document const& doc, int index) noexcept
    : _doc(doc)
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
    } fz_catch(_doc._ctx) {
        // [[unlikely]] 一般不会进入这里
    }
}

QSizeF Page::size() const {
    auto rect = fz_bound_page(_doc._ctx, _page);
    return {rect.x1 - rect.x0, rect.y1 - rect.y0};
}

QImage Page::renderImage(float scaleX, float scaleY, float rotation) const {
    fz_pixmap* pixmap = nullptr;
    unsigned char* samples = nullptr;
    unsigned char* copyed_samples = nullptr;
    int width = 0;
    int height = 0;
    int size = 0;

    fz_rect mediabox;
    fz_stext_page* text_page = fz_new_stext_page(_doc._ctx, fz_bound_page(_doc._ctx, _page));
    fz_try(_doc._ctx) {
        fz_device* tdev = fz_new_stext_device(_doc._ctx, text_page, nullptr);
        fz_run_display_list(_doc._ctx, _displayList, tdev, fz_identity, fz_infinite_rect, nullptr);
        fz_close_device(_doc._ctx, tdev);
        fz_drop_device(_doc._ctx, tdev);
    } fz_catch(_doc._ctx) {
        fz_drop_stext_page(_doc._ctx, text_page);
        return QImage();
    }

    // build transform matrix
    fz_matrix transform = fz_pre_rotate(fz_scale(scaleX, scaleY), rotation);
    
    // get transformed page size
    fz_rect bounds = fz_bound_page(_doc._ctx, _page);
    fz_irect bbox = fz_round_rect(fz_transform_rect(bounds, transform));
    bounds = fz_rect_from_irect(bbox);

    // render to pixmap
    fz_device* dev = nullptr;
    fz_try(_doc._ctx) {
        pixmap = fz_new_pixmap_with_bbox(_doc._ctx, fz_device_rgb(_doc._ctx), bbox, nullptr, 1);

        if (!_doc._transparent) {
            samples = fz_pixmap_samples(_doc._ctx, pixmap);
            if (!samples) {
                fz_throw(_doc._ctx, FZ_ERROR_GENERIC, "Failed to get pixmap samples");
            }
            clear_rgb_samples_with_value(samples, size, _doc._b, _doc._g, _doc._r, _doc._a);
        }
        
        dev = fz_new_draw_device(_doc._ctx, transform, pixmap);
        fz_run_display_list(_doc._ctx, _displayList, dev, transform, bounds, nullptr);
        
        samples = fz_pixmap_samples(_doc._ctx, pixmap);
        width = fz_pixmap_width(_doc._ctx, pixmap);
        height = fz_pixmap_height(_doc._ctx, pixmap);
        size = width * height * 4;
    } fz_always(_doc._ctx) {
        if (dev) {
            fz_close_device(_doc._ctx, dev);
            fz_drop_device(_doc._ctx, dev);
        }
    } fz_catch(_doc._ctx) {
        if (pixmap) {
            fz_drop_pixmap(_doc._ctx, pixmap);
        }
        fz_drop_stext_page(_doc._ctx, text_page);
        return QImage();
    }
    
    // render to QImage
    QImage image;
    if (!pixmap) {
        fz_drop_stext_page(_doc._ctx, text_page);
        return image;
    }
    
    copyed_samples = new unsigned char[size];
    memcpy(copyed_samples, samples, size);
    fz_drop_pixmap(_doc._ctx, pixmap);
    
    image = QImage(copyed_samples, width, height, QImage::Format_RGBA8888,
                   [](void* data) { delete[] static_cast<unsigned char*>(data); }, copyed_samples);
    
    fz_drop_stext_page(_doc._ctx, text_page);
    return image;
}

/*
QImage Page::renderImage(float scaleX, float scaleY, float rotation) const {
    fz_pixmap* pixmap = nullptr;
    unsigned char* samples = nullptr;
    unsigned char* copyed_samples = nullptr;
    int width = 0;
    int height = 0;
    int size = 0;

    fz_rect mediabox;
    fz_stext_page* text_page = fz_new_stext_page(_doc._ctx, fz_bound_page(_doc._ctx, _page));

    fz_device* tdev;
    tdev = fz_new_stext_device(_doc._ctx, text_page, nullptr);
    fz_run_display_list(_doc._ctx, _displayList, tdev, fz_identity, fz_infinite_rect, nullptr);
    fz_close_device(_doc._ctx, tdev);
    fz_drop_device(_doc._ctx, tdev);

    // build transform matrix
    fz_matrix transform = fz_pre_rotate(fz_scale(scaleX, scaleY), rotation);

    // get transformed page size
    fz_rect bounds = fz_bound_page(_doc._ctx, _page);
    fz_irect bbox = fz_round_rect(fz_transform_rect(bounds, transform));
    bounds = fz_rect_from_irect(bbox);

    // render to pixmap
    fz_device* dev = nullptr;
    fz_try(_doc._ctx) {
        // fz_pixmap will always include a separate alpha channel
        pixmap = fz_new_pixmap_with_bbox(_doc._ctx, fz_device_rgb(_doc._ctx), bbox, nullptr, 1);

        if (!_doc._transparent) {
            if (_doc._b == 255 && _doc._g == 255 && _doc._r == 255 && _doc._a == 255) {
                // with white background
                fz_clear_pixmap_with_value(_doc._ctx, pixmap, 0xff);
            } else {
                // with user defined background color
                clear_rgb_samples_with_value(samples, size, _doc._b, _doc._g, _doc._r, _doc._a);
            }
        }
        dev = fz_new_draw_device(_doc._ctx, {}, pixmap);
        fz_run_display_list(_doc._ctx, _displayList, dev, transform, bounds, nullptr);

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
    } fz_catch(_doc._ctx) {
        if (pixmap) {
            fz_drop_pixmap(_doc._ctx, pixmap);
        }
        pixmap = nullptr;
    }

    // render to QImage
    QImage image;
    if (!pixmap) {
        return image;
    }
    copyed_samples = new unsigned char[size];
    memcpy(copyed_samples, samples, size);
    fz_drop_pixmap(_doc._ctx, pixmap);

    image = QImage(copyed_samples,
                   width, height, QImage::Format_RGBA8888,
                   imageCleanupHandler, copyed_samples);

    return image;
}
    */

Page::~Page() noexcept {
    if (_displayList) [[likely]] {
        fz_drop_display_list(_doc._ctx, _displayList);
    }
    if (_page) [[likely]] {
        fz_drop_page(_doc._ctx, _page);
    }
}

} // namespace HX::Mu