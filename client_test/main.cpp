#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <csignal>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

asio::io_context ioc;
tcp::resolver resolver(ioc);
websocket::stream<tcp::socket> ws(ioc);

// 处理 Ctrl+C 事件
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nCtrl+C detected. Closing WebSocket connection..." << std::endl;
        if (ws.is_open()) {
            boost::system::error_code ec;
            ws.close(websocket::close_code::normal, ec);
            if (ec) {
                std::cerr << "Error closing WebSocket: " << ec.message() << std::endl;
            }
        }
        ioc.stop();  // 停止 io_context
        exit(0);
    }
}

void websocket_client(const std::string& host, const std::string& port, const std::string& client_id) {
    try {

        signal(SIGINT, signal_handler);
        // 解析地址并连接
        auto const results = resolver.resolve(host, port);
        asio::connect(ws.next_layer(), results.begin(), results.end());
        
        // WebSocket 握手
        ws.handshake(host, "/");
        
        // 发送注册信息
        json reg_msg = {
            {"type", "register"},
            {"id", client_id}
        };
        ws.write(asio::buffer(reg_msg.dump()));
        std::cout << "Registered as " << client_id << std::endl;
        
        // 发送测试消息
        std::string target_id;
        std::cout << "Enter target client ID: ";
        std::cin >> target_id;

        while (true) {
            std::string message;
            std::cout << "Enter message: ";
            std::cin>>message;

            json msg = {
                {"type", "message"},
                {"target", target_id},
                {"content", message}
            };
            std::cout<<message<<std::endl;
            ws.write(asio::buffer(msg.dump()));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "WebSocket Client Error: " << e.what() << std::endl;
    }
}

int main() {
    std::string host = "127.0.0.1";
    std::string port = "8888";
    std::string client_id;
    
    std::cout << "Enter your client ID: ";
    std::cin >> client_id;
    
    websocket_client(host, port, client_id);
    
    return 0;
}
