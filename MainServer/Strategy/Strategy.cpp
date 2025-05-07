#include "Strategy.h"
#include <iostream>
#include "Server.h"
// 初始化静态成员
std::unordered_map<std::string, std::function<Strategy*()>> Strategy::type_create_map;

// 注册策略
void Strategy::registerStrategy(const std::string& type, 
                              std::function<Strategy*()> creator) {
    type_create_map[type] = creator;
}

// 静态初始化注册策略
namespace {
    struct StrategyInitializer {
        StrategyInitializer() {
            Strategy::registerStrategy("message", []() { return new MessageStrategy(); });
            Strategy::registerStrategy("get_target_status", []() { return new GetTargetStatusStrategy(); });
            Strategy::registerStrategy("connect_request", []() { return new ConnectRequestStrategy(); });
            Strategy::registerStrategy("connect_request_result", []() { return new ConnectRequestResultStrategy(); });
            Strategy::registerStrategy("ready", []() { return new ReadyStrategy(); });
            Strategy::registerStrategy("sdp_offer", []() { return new SdpOfferStrategy(); });
            Strategy::registerStrategy("sdp_answer", []() { return new SdpAnswerStrategy(); });
            Strategy::registerStrategy("ice_candidate", []() { return new IceCanDidateStrategy(); });
            Strategy::registerStrategy("ice_gather_done", []() { return new IceGatherDoneStrategy(); });
            Strategy::registerStrategy("logout", []() { return new LogOutStrategy(); });
        }
    };
    StrategyInitializer initializer;
}

// 获取策略实例
std::unique_ptr<Strategy> Strategy::getStrategy(std::string&& type) {
    auto it = type_create_map.find(type);
    if (it != type_create_map.end()) {
        return std::unique_ptr<Strategy>(it->second());
    }
    return nullptr;
}

void MessageStrategy::run(json msg) {
    if(msg.contains("message") && msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        if(Server::getInstance().hasID(user_id) && Server::getInstance().hasID(target_id))
        {
            json dispath_msg = {{"id",user_id},{"content",{{"message",msg["message"]}}}};
            Server::getInstance().send_to_client(target_id, dispath_msg);
        }
        else
        {
            std::cout<<"target host is off-line"<<std::endl;
        }
    }
}

void GetTargetStatusStrategy::run(json msg) {
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        if(Server::getInstance().hasID(user_id))
        {
            std::string status;
            if(Server::getInstance().hasID(target_id))
            {
                status = "True";
            }
            else
            {
                status = "False";
            }
            json response_json = {{"type","target_status"},{"content",{{"status",status}}}};
            Server::getInstance().send_to_client(user_id,response_json.dump());
        }
        else
        {
            std::cout<<"Illegal User Id"<<std::endl;
        }
    }
}

void ConnectRequestStrategy::run(json msg)
{
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        json response_json = {{"type","connect_request"},{"content",{{"target_id",user_id}}}};
        Server::getInstance().send_to_client(target_id,response_json.dump());
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}

void ConnectRequestResultStrategy::run(json msg)
{
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        json response_json = {{"type","connect_request_result"},{"content",{{"target_id",user_id},{"result",msg["result"]}}}};
        Server::getInstance().send_to_client(target_id,response_json.dump());
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}

void SdpOfferStrategy::run(json msg)
{
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        json response_json = {{"type","sdp_offer"},{"content",{{"target_id",user_id},{"sdp",msg["sdp"]}}}};
        Server::getInstance().send_to_client(target_id,response_json.dump());
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}

void SdpAnswerStrategy::run(json msg)
{
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        json response_json = {{"type","sdp_answer"},{"content",{{"target_id",user_id},{"sdp",msg["sdp"]}}}};
        Server::getInstance().send_to_client(target_id,response_json.dump());
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}

void IceCanDidateStrategy::run(json msg)
{
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        json response_json = {{"type","ice_candidate"},{"content",{{"target_id",user_id},{"ice_content",msg["ice_content"]}}}};
        Server::getInstance().send_to_client(target_id,response_json.dump());
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}

void IceGatherDoneStrategy::run(json msg)
{
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        json response_json = {{"type","ice_gather_done"},{"content",{{"target_id",user_id}}}};
        Server::getInstance().send_to_client(target_id,response_json.dump());
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}

void ReadyStrategy::run(json msg)
{
    if(msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        json response_json = {{"type","ready"},{"content",{{"target_id",user_id}}}};
        Server::getInstance().send_to_client(target_id,response_json.dump());
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}

void LogOutStrategy::run(json msg)
{
    if(msg.contains("user_id"))
    {
        std::string user_id = msg["user_id"];
        Server::getInstance().closeClient(user_id);
    }
    else
    {
        std::cout<<"Illegal User Id"<<std::endl;
    }
}