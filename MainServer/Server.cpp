#include "Server.h"
#include "MessageParser.h"



Server::Server() 
    : parser(std::make_unique<MessageParser>()),
      acceptor(ioc)
{}

Server::~Server() {}

void Server::startServer() {
    tcp::endpoint endpoint(tcp::v4(), 8888);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();

    std::cout << "Server started on port 8888" << std::endl;

    do_accept();
    ioc.run();  // 启动事件循环
}

void Server::do_accept() {
    acceptor.async_accept([this](system::error_code ec, tcp::socket socket) {
        if (!ec) {
            auto client_ws = std::make_shared<websocket>(std::move(socket));
            client_ws->async_accept([this, client_ws](system::error_code ec2) {
                if (!ec2) {
                    std::string tmp_id = "tmp_" + std::to_string(reinterpret_cast<std::uintptr_t>(client_ws.get()));
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex);
                        id_clients[tmp_id] = client_ws;
                        std::cout << "New client connected: " << tmp_id << std::endl;
                    }
                    async_read_message(client_ws);
                } else {
                    std::cerr << "Handshake failed: " << ec2.message() << std::endl;
                }
            });
        } else {
            std::cerr << "Accept error: " << ec.message() << std::endl;
        }

        // 等待下一个连接
        do_accept();
    });
}

void Server::async_read_message(std::shared_ptr<websocket> client_ws) {
    auto buffer = std::make_shared<beast::flat_buffer>();

    client_ws->async_read(*buffer, [this, client_ws, buffer](system::error_code ec, std::size_t bytes_transferred) {
        if (ec) {
            std::cerr << "Read error: " << ec.message() << std::endl;
            return;
        }

        std::string message = beast::buffers_to_string(buffer->data());
        buffer->consume(buffer->size());
        std::cout << "Received: " << message << std::endl;

        try {
            json msg = json::parse(message);

            if (msg.contains("type") && msg.contains("content")) {
                if (msg["type"] == "register") {
                    std::string client_id = msg["content"]["id"];
                    std::string tmp_id = "tmp_" + std::to_string(reinterpret_cast<std::uintptr_t>(client_ws.get()));
                    {
                        std::lock_guard<std::mutex> lock(clients_mutex);
                        if (id_clients.find(tmp_id) != id_clients.end()) {
                            id_clients.erase(tmp_id);
                        }
                        id_clients[client_id] = client_ws;
                    }
                    json response = {
                        {"type", "register_result"},
                        {"content", {{"status", "success"}}}
                    };
                    client_ws->async_write(asio::buffer(response.dump()), [](system::error_code, std::size_t){});
                } else {
                    parser->parseMsg(std::move(msg["type"]), std::move(msg["content"]));
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }

        // 持续读取后续消息
        async_read_message(client_ws);
    });
}

// void Server::send_to_client(const std::string& target_id, const std::string& message) {
//     std::lock_guard<std::mutex> lock(clients_mutex);
//     if (id_clients.find(target_id) != id_clients.end()) {
//         auto ws = id_clients[target_id];
//         if (ws->is_open()) {
//             ws->async_write(asio::buffer(message), [target_id](system::error_code ec, std::size_t) {
//                 if (ec) {
//                     std::cerr << "Failed to send to " << target_id << ": " << ec.message() << std::endl;
//                 } else {
//                     std::cout << "Sent to " << target_id << std::endl;
//                 }
//             });
//         } else {
//             std::cerr << "Client " << target_id << " is not open!" << std::endl;
//         }
//     } else {
//         std::cerr << "Client " << target_id << " not found!" << std::endl;
//     }
// }

void Server::send_to_client(const std::string& target_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (id_clients.find(target_id) != id_clients.end()) {
        auto ws = id_clients[target_id];
        if (ws->is_open()) {
            // 获取或创建发送队列
            auto& queue = send_queues[target_id];
            
            // 将消息加入队列
            queue.push(message);
            
            // 如果队列中只有这一个消息，开始发送
            if (queue.size() == 1) {
                start_sending(target_id, ws);
            }
        }
    }
}

void Server::start_sending(const std::string& target_id, std::shared_ptr<websocket> ws) {
    auto& queue = send_queues[target_id];
    
    ws->async_write(
        asio::buffer(queue.front()),
        [this, target_id, ws](system::error_code ec, std::size_t) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            auto& queue = send_queues[target_id];
            
            if (ec) {
                std::cerr << "Failed to send to " << target_id << ": " << ec.message() << std::endl;
                // 清除队列
                std::queue<std::string>().swap(queue);
                return;
            }
            
            // 移除已发送的消息
            queue.pop();
            
            // 如果还有消息，继续发送
            if (!queue.empty()) {
                start_sending(target_id, ws);
            }
        });
}
