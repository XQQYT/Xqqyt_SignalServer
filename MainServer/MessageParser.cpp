/*
 * Xqqyt_SignalServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#include "MessageParser.h"
#include <iostream>
//json example
/*
{
    "type": "resigter",
    "content": {
        "status": "OK"
    }
}
*/
MessageParser::MessageParser()
{

}
MessageParser::~MessageParser()
{

}
void MessageParser::parseMsg(std::string&& type,json&& msg)
{
    auto cur_strategy = Strategy::getStrategy(std::move(type));
    if(cur_strategy)
        cur_strategy->run(std::move(msg));
    else
    {
        std::cout<<"Don't support this type : "<<type<<std::endl;
    }
}