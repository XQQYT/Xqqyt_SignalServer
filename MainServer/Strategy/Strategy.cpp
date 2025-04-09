#include "Strategy.h"
#include <iostream>
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
    std::cout<<"strategy   "<<msg<<std::endl;
}