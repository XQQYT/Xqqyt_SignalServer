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

std::unordered_map<std::string, std::shared_ptr<websocket::stream<tcp::socket>>> id_clients;
std::unordered_map<int,std::shared_ptr<websocket::stream<tcp::socket>>> sd_clients;
std::mutex clients_mutex;
asio::io_context ioc;

void send_to_client(const std::string& target_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (id_clients.find(target_id) != id_clients.end()) {
        id_clients[target_id]->write(asio::buffer(message));
    } else {
        std::cerr << "Client " << target_id << " not found!" << std::endl;
    }
}

//处理客户端连接请求，并建立websocket连接 
void accept_and_ws_shakehand(int listen_fd,int epoll)
{
    int client_fd=accept(listen_fd,NULL,0);
    try{
    tcp::socket client_sd(ioc);
    client_sd.assign(tcp::v4(),client_fd);
    auto client_ws=std::make_shared<websocket::stream<tcp::socket>>(std::move(client_sd));
    client_ws->accept();

    auto client_tmp_id="tmp_"+std::to_string(client_fd);
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        id_clients[client_tmp_id]=client_ws;
        sd_clients[client_fd] = client_ws;
        std::cout<<"add client with tmp id "<<client_tmp_id<<std::endl;
    }
    }
    catch (const std::exception& e){
        std::cout<<e.what()<<std::endl;
    }

    struct epoll_event cur_fd_ev;
    cur_fd_ev.events=EPOLLIN | EPOLLET;
    cur_fd_ev.data.fd=client_fd;
    epoll_ctl(epoll,EPOLL_CTL_ADD,client_fd,&cur_fd_ev);
}

//处理客户端消息
void deal_client_msg(int client_socket) {
    try {
        
        if(sd_clients.find(client_socket) == sd_clients.end())
        {
            std::cout<<"client has not shaked with server"<<std::endl;
            return;
        }
        auto client_ws = sd_clients[client_socket];
        std::string client_id = "";
        beast::flat_buffer buffer;
        client_ws->read(buffer);

        std::string received_message = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size()); // 清空 buffer
        std::cout<<"rec client message  "<<received_message<<std::endl;
        json msg = json::parse(received_message);
        if (msg["type"] == "register") {
            client_id = msg["id"];
            {
                auto need_to_record_client_ws = client_ws;
                std::string tmp_id = "tmp_" + std::to_string(client_socket);
                std::lock_guard<std::mutex> lock(clients_mutex);
                if(id_clients.find(tmp_id) != id_clients.end())
                {                    
                    need_to_record_client_ws = id_clients[tmp_id];
                    id_clients.erase(tmp_id);
                }
                id_clients[client_id]=need_to_record_client_ws;
            }
            std::cout << "Client registered: " << client_id << std::endl;
        } else {
            std::string target_id = msg["target"];
            //send_to_client(target_id, received_message);
            std::cout<<received_message<<std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
    }
}

int main() {
    int listen_socket=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in listen_addr;
    listen_addr.sin_addr.s_addr=INADDR_ANY;
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(8888);
    bind(listen_socket,(sockaddr*)&listen_addr,sizeof(listen_addr));

    int epoll=epoll_create(256);
    struct epoll_event listen_ev;
    listen_ev.events=EPOLLIN | EPOLLET;
    listen_ev.data.fd=listen_socket;

    epoll_ctl(epoll,EPOLL_CTL_ADD,listen_socket,&listen_ev);
    listen(listen_socket,-1);

    epoll_event envs[256];
    while(true)
    {
        int ready_num=epoll_wait(epoll,envs,sizeof(envs)/sizeof(epoll_event),-1);
        for(int i=0;i<ready_num;i++)
        {
            int cur_fd=envs[i].data.fd;
            if(cur_fd==listen_socket)
            {
                std::cout<<"have new client"<<std::endl;
                accept_and_ws_shakehand(listen_socket,epoll);
            }
            else
            {
                std::cout<<"have client mseeage"<<std::endl;
                std::thread(deal_client_msg, std::move(cur_fd)).detach();
            }
        }
    }

    return 0;
}
