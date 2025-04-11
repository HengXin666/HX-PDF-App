#include <HXWeb/HXApi.hpp>

#include <HXJson/JsonWrite.hpp>
#include <HXJson/JsonRead.hpp>

struct ServerConfig {
    std::string pdfPath;
} serverConfig;

class PdfTestController {
    ROUTER
        .on<GET>("/", [] ENDPOINT {
            RESPONSE_DATA(200, html, R"(
                <h1>这里是HX-PDF-CPP后端</h1>
                <h2>基于<a href="https://github.com/HengXin666/HXLibs">HXLibs</a>高性能Http服务器框架开发</h2>)");
            co_return;
        })
        .on<GET, HEAD>("/files/**", [] ENDPOINT {
            PARSE_MULTI_LEVEL_PARAM(uwp);
            try {
                // todo: 需要补充完整路径!
                co_await res.useRangeTransferFile(serverConfig.pdfPath + uwp);
            } catch (std::exception const& ec) {
                RESPONSE_DATA(500, html, "<h1>文件不存在!</h1>");
                LOG_ERROR(ec.what());
            }
        })
    ROUTER_END;
};

#include <fstream>
#include <filesystem>

int main() {
    {
        try {
            auto cwd = std::filesystem::current_path();
            std::cout << "当前工作路径是: " << cwd << '\n';
            std::filesystem::current_path("../cpp-backend/config");
            std::cout << "切换到配置文件路径: " << std::filesystem::current_path() << '\n';
        
            std::ifstream is{"server.json", std::ios::in};

            std::string buf;
            buf.resize(1024);
            
            is.read(buf.data(), 114514);
            HX::json::fromJson(serverConfig, buf);
            HX::print::println("服务器配置已加载: ", serverConfig);

            std::filesystem::current_path(serverConfig.pdfPath);
            std::cout << "当前路径: " << std::filesystem::current_path() << '\n';
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << '\n';
        }
    }
    setlocale(LC_ALL, "zh_CN.UTF-8");
    ROUTER_BIND(PdfTestController);
    // 启动Http服务 [阻塞于此]
    HX::web::server::ServerRun::startHttp("127.0.0.1", "28205");
    return 0;
}