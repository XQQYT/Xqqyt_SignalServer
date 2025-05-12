/*
 * Xqqyt_SignalServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#ifndef _MESSAGEPARSER_H
#define _MESSAGEPARSER_H

#include <string>
#include <nlohmann/json.hpp>
#include "Strategy.h"
using json = nlohmann::json;

class MessageParser{
public:
    MessageParser();
    ~MessageParser();
    void parseMsg(std::string&& type,json&& msg);

};

#endif