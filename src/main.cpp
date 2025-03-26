#include <QApplication>
#include <window/MainWindow.h>

#include <mupdf/pdf.h>

int _1main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    HX::MainWindow window;
    window.show();
    return app.exec();
}

#include <QApplication>
#include <QLabel>
#include <QImage>
#include <mupdf/fitz.h>

// 将 fz_pixmap 转换为 QImage
QImage fzPixmapToQImage(fz_pixmap *pix)
{
    if (!pix)
        return QImage();

    int width = pix->w;
    int height = pix->h;
    int n = pix->n; // 通道数 (RGB:3, RGBA:4)
    
    QImage img(width, height, QImage::Format_RGB888);

    unsigned char *samples = pix->samples;
    for (int y = 0; y < height; y++)
    {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < width; x++)
        {
            int r = samples[0];
            int g = samples[1];
            int b = samples[2];

            line[x] = qRgb(r, g, b);
            samples += n; // 移动到下一个像素
        }
    }

    return img;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const char *filename = "D:\\command\\Github\\HX-PDF-App\\TestPdfSrc\\C++-Templates-The-Complete-Guide-zh-20220903.pdf"; // 替换为你的 PDF 文件路径
    int page_number = 0; // 0 代表第一页
    float zoom = 100.0f;
    float rotate = 0.0f;

    // 创建 MuPDF 上下文
    fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!ctx)
    {
        qCritical("无法创建 MuPDF 上下文");
        return EXIT_FAILURE;
    }

    // 注册文档处理器
    fz_register_document_handlers(ctx);

    // 打开 PDF 文档
    fz_document *doc = fz_open_document(ctx, filename);
    if (!doc)
    {
        qCritical("无法打开文档");
        fz_drop_context(ctx);
        return EXIT_FAILURE;
    }

    // 获取 PDF 总页数
    int page_count = fz_count_pages(ctx, doc);
    if (page_number < 0 || page_number >= page_count)
    {
        qCritical("页码超出范围");
        fz_drop_document(ctx, doc);
        fz_drop_context(ctx);
        return EXIT_FAILURE;
    }

    // 计算缩放和旋转矩阵
    fz_matrix ctm = fz_scale(zoom / 100.0f, zoom / 100.0f);
    ctm = fz_pre_rotate(ctm, rotate);

    // 渲染指定页到 fz_pixmap
    fz_pixmap *pix = fz_new_pixmap_from_page_number(ctx, doc, page_number, ctm, fz_device_rgb(ctx), 0);
    if (!pix)
    {
        qCritical("无法渲染页面");
        fz_drop_document(ctx, doc);
        fz_drop_context(ctx);
        return EXIT_FAILURE;
    }

    // 转换为 QImage
    QImage img = fzPixmapToQImage(pix);

    // 显示图片
    QLabel label;
    label.setPixmap(QPixmap::fromImage(img));
    label.show();

    // 释放资源
    fz_drop_pixmap(ctx, pix);
    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);

    return app.exec();
}
