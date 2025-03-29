#include <HXWeb/HXApi.hpp>

class PdfTestController {
    ROUTER
        .on<GET>("/", [] ENDPOINT {
            RESPONSE_DATA(200, html, R"(
                <h1>这里是HX-PDF-CPP后端</h1>
                <h2>基于<a href="https://github.com/HengXin666/HXLibs">HXLibs</a>高性能Http服务器框架开发</h2>)");
            co_return;
        })
        .on<GET>("/files/**", [] ENDPOINT {
            PARSE_MULTI_LEVEL_PARAM(uwp);
            try {
                // todo: 需要补充完整路径!
                co_await res.useRangeTransferFile(uwp);
            } catch (std::exception const& ec) {
                RESPONSE_DATA(500, html, "<h1>文件不存在!</h1>");
                LOG_ERROR(ec.what());
            }
        })
    ROUTER_END;
};

int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    ROUTER_BIND(PdfTestController);
    // 启动Http服务 [阻塞于此]
    HX::web::server::ServerRun::startHttp("127.0.0.1", "28205");
    return 0;
}