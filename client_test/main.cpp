#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <thread>
#include <csignal>
#include <memory>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

asio::io_context ioc;
tcp::resolver resolver(ioc);
std::shared_ptr<websocket::stream<tcp::socket>> ws;  // 共享 WebSocket

// 处理 Ctrl+C
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nCtrl+C detected. Closing WebSocket connection..." << std::endl;
        if (ws && ws->is_open()) {
            boost::system::error_code ec;
            ws->close(websocket::close_code::normal, ec);
            if (ec) {
                std::cerr << "Error closing WebSocket: " << ec.message() << std::endl;
            }
        }
        ioc.stop();
        exit(0);
    }
}

// 线程：持续接收 WebSocket 消息
void receive_messages() {
    try {
        while (ws && ws->is_open()) {
            beast::flat_buffer buffer;
            boost::system::error_code ec;
            ws->read(buffer, ec);
            
            if (ec == websocket::error::closed) {
                std::cerr << "WebSocket closed by server." << std::endl;
                return;
            } else if (ec) {
                std::cerr << "Receive Error: " << ec.message() << std::endl;
                return;
            }

            std::string received_message = beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());  // 清空缓冲区

            std::cout << "\n[Received] " << received_message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Receive Thread Exception: " << e.what() << std::endl;
    }
}

// 线程：负责连接 & 发送消息
void websocket_client(const std::string& host, const std::string& port, const std::string& client_id) {
    try {
        signal(SIGINT, signal_handler);

        // 解析地址并连接
        auto const results = resolver.resolve(host, port);
        ws = std::make_shared<websocket::stream<tcp::socket>>(ioc);
        asio::connect(ws->next_layer(), results.begin(), results.end());

        // WebSocket 握手
        ws->handshake(host, "/");

        // 发送注册信息
        json reg_msg = {{"type", "register"}, {"id", client_id}};
        ws->write(asio::buffer(reg_msg.dump()));
        std::cout << "Registered as " << client_id << std::endl;

        // 启动接收线程
        std::thread receiver_thread(receive_messages);
        receiver_thread.detach();  // 独立运行

        // 读取目标客户端 ID
        std::string target_id;
        std::cout << "Enter target client ID: ";
        std::getline(std::cin >> std::ws, target_id);  // 处理输入流残留换行符

        while (true) {
            std::string message;
            std::cout << "Enter message: ";
            std::getline(std::cin, message);  // 确保读取完整消息

            if (message == "exit") {
                std::cout << "Exiting chat..." << std::endl;
                break;
            }

            json msg = {{"type", "message"}, {"target", target_id}, {"content", message}};
            ws->write(asio::buffer(msg.dump()));
        }

        // 关闭 WebSocket
        if (ws->is_open()) {
            ws->close(websocket::close_code::normal);
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
    std::getline(std::cin >> std::ws, client_id);  // 处理输入流残留换行符

    websocket_client(host, port, client_id);
    
    return 0;
}
