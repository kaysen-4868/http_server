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

    //请求头最大字节数(含请求行，所有头，空行)
    static const size_t MAX_HEADER_SIZE=8*1024;//8kb

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
    size_t header_bytes_=0;//已消费的头部字节数
};