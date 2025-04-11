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

void MessageStrategy::run(json&& msg) {
    if(msg.contains("message") && msg.contains("user_id") && msg.contains("target_id"))
    {
        std::string user_id = msg["user_id"];
        std::string target_id = msg["target_id"];
        if(Server::getInstance().hasID(user_id) && Server::getInstance().hasID(target_id))
        {
            Server::getInstance().send_to_client(target_id,msg["message"]);
        }
        else
        {
            std::cout<<"target host is off-line"<<std::endl;
        }
    }
}

void GetTargetStatusStrategy::run(json&& msg) {
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