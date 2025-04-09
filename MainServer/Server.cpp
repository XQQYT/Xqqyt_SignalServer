#include "Server.h"

void Server::send_to_client(const std::string& target_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (id_clients.find(target_id) != id_clients.end()) {
        id_clients[target_id]->write(asio::buffer(message));
        std::cout<<"send to "<<target_id<<"  "<<message<<std::endl;
    } else {
        std::cerr << "Client " << target_id << " not found!" << std::endl;
    }
}

//处理客户端连接请求，并建立websocket连接 
void Server::accept_and_ws_shakehand(int listen_fd,int epoll)
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
void Server::deal_client_msg(int client_socket) {
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

        if(msg.contains("type") && msg.contains("content"))
            if (msg["type"] == "register") 
            {
                client_id = msg["content"]["id"];
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
                    client_ws->write(asio::buffer("done"));
                }
            }
            else
            {
                parser.parseMsg(std::move(msg["type"]), std::move(msg["content"]));
            }
    } catch (const std::exception& e) {
        std::cerr << "WebSocket error: " << e.what() << std::endl;
    }
}

void Server::startServer() {
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
    std::cout<<"Server is running"<<std::endl;
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
                std::thread(&Server::deal_client_msg,this, cur_fd).detach();
            }
        }
    }
}
