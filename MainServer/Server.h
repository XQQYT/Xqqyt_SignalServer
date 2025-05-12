/*
 * Xqqyt_SignalServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>
#include <queue>

using tcp = boost::asio::ip::tcp;
using websocket = boost::beast::websocket::stream<tcp::socket>;
using namespace boost;
using json = nlohmann::json;
class MessageParser;

class Server {
public:
    
    ~Server();
    static Server& getInstance()
    {
        static Server instance;
        return instance;
    }
    void startServer();
    void send_to_client(const std::string& target_id, const std::string& message);
    inline bool hasID(const std::string& id)
    {
        return id_clients.find(id) != id_clients.end();
    }
    inline std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<websocket>>> get_id_clients()
    {
        return std::make_shared<std::unordered_map<std::string, std::shared_ptr<websocket>>>(id_clients);
    }
    // inline std::shared_ptr<std::unordered_map<int,std::shared_ptr<websocket>>> get_sd_clients()
    // {
    //     return std::make_shared<std::unordered_map<int,std::shared_ptr<websocket>>>(sd_clients);
    // }
    void start_sending(const std::string& target_id, std::shared_ptr<websocket> ws);
    void closeClient(const std::string& id);
private:
    Server();


    boost::asio::io_context ioc;
    tcp::acceptor acceptor;

    std::unordered_map<std::string, std::shared_ptr<websocket>> id_clients;
    std::mutex clients_mutex;

    std::unique_ptr<MessageParser> parser;

    void do_accept();
    void async_read_message(std::shared_ptr<websocket> client_ws);
    private:
    std::unordered_map<std::string, std::queue<std::string>> send_queues;
};
