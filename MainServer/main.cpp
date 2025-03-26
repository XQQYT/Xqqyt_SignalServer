#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

std::unordered_map<std::string, std::shared_ptr<websocket::stream<tcp::socket>>> clients;
std::mutex clients_mutex;

void send_to_client(const std::string& target_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (clients.find(target_id) != clients.end()) {
        clients[target_id]->write(asio::buffer(message));
    } else {
        std::cerr << "Client " << target_id << " not found!" << std::endl;
    }
}

void do_session(tcp::socket socket) {
    try {
        websocket::stream<tcp::socket> ws(std::move(socket));
        ws.accept();

        std::string client_id = "";
        beast::flat_buffer buffer;
        
        while (true) {
            if(clients.find(client_id)==clients.end())
                ws.read(buffer);
            else
                clients[client_id]->read(buffer);
            std::string received_message = beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size()); // 清空 buffer
            json msg = json::parse(received_message);
            if (msg["type"] == "register") {
                client_id = msg["id"];
                {
                    std::lock_guard<std::mutex> lock(clients_mutex);
                    clients[client_id] = std::make_shared<websocket::stream<tcp::socket>>(std::move(ws));;
                }
                std::cout << "Client registered: " << client_id << std::endl;
            } else {
                std::string target_id = msg["target"];
                //send_to_client(target_id, received_message);
                std::cout<<received_message<<std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        asio::io_context ioc;
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8888));

        while (true) {
            tcp::socket socket(ioc);
            acceptor.accept(socket);
            std::thread(do_session, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    return 0;
}
