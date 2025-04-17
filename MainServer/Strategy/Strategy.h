#ifndef _STRATEGY_H
#define _STRATEGY_H

#include <nlohmann/json.hpp>
#include <functional>
#include <unordered_map>
#include <memory> 
using json = nlohmann::json;

class Server;

class Strategy {
public:
    Strategy() = default;
    virtual void run(json msg) = 0;
    virtual ~Strategy() = default;
    
    static std::unique_ptr<Strategy> getStrategy(std::string&& type);
    
    // 注册策略创建函数
    static void registerStrategy(const std::string& type, 
                               std::function<Strategy*()> creator);

private:
    static std::unordered_map<std::string, std::function<Strategy*()>> type_create_map;
};

class MessageStrategy : public Strategy {
public: 
    MessageStrategy() = default;
    void run(json msg) override;
    ~MessageStrategy() = default;
};

class GetTargetStatusStrategy : public Strategy {
public: 
    GetTargetStatusStrategy() = default;
    void run(json msg) override;
    ~GetTargetStatusStrategy() = default;
};

class ConnectRequestStrategy : public Strategy {
public: 
    ConnectRequestStrategy() = default;
    void run(json msg) override;
    ~ConnectRequestStrategy() = default;
};

class ConnectRequestResultStrategy : public Strategy {
public: 
    ConnectRequestResultStrategy() = default;
    void run(json msg) override;
    ~ConnectRequestResultStrategy() = default;
};

#endif