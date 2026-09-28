#pragma once

#include "http/HttpRequest.h"
#include<string>

class HttpParser
{
    public:
    enum class State
    {
        REQUEST_LINE,
        HEADERS,
        COMPLETE,
        ERROR
    };

    bool parse(std::string& buf);
    void reset();
    
    State state()const{return state_;}
    const HttpRequest& request()const{return request_;}

    

    private:
    static bool extractLine(std::string& buf,std::string& line);
    bool parseRequestLine(const std::string& line);
    bool parseHeaderLine(const std::string& line);

    State state_ =State::REQUEST_LINE;
    HttpRequest request_;
};