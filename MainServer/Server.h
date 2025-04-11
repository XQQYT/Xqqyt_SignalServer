#ifndef _SERVER_H
#define _SERVER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sys/epoll.h>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

class MessageParser;

class Server{
public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    ~Server();
    static Server& getInstance()
    {
        static Server instance;
        return instance;
    }
    void startServer();
    void send_to_client(const std::string& target_id, const std::string& message);
    
    //处理客户端连接请求，并建立websocket连接 
    void accept_and_ws_shakehand(int listen_fd,int epoll);
    //处理客户端消息
    void deal_client_msg(int client_socket);
    inline bool hasID(const std::string& id)
    {
        return id_clients.find(id) != id_clients.end();
    }
    inline std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<websocket::stream<tcp::socket>>>> get_id_clients()
    {
        return std::make_shared<std::unordered_map<std::string, std::shared_ptr<websocket::stream<tcp::socket>>>>(id_clients);
    }
    inline std::shared_ptr<std::unordered_map<int,std::shared_ptr<websocket::stream<tcp::socket>>>> get_sd_clients()
    {
        return std::make_shared<std::unordered_map<int,std::shared_ptr<websocket::stream<tcp::socket>>>>(sd_clients);
    }
private:
    Server();
    std::unordered_map<std::string, std::shared_ptr<websocket::stream<tcp::socket>>> id_clients;
    std::unordered_map<int,std::shared_ptr<websocket::stream<tcp::socket>>> sd_clients;
    std::mutex clients_mutex;
    asio::io_context ioc;

    std::unique_ptr<MessageParser> parser;
};

#endif