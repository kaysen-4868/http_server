#pragma once

#include "http/HttpParser.h"

class HttpContext
{
    public:
    bool parseRequest(std::string& buf){return parser_.parse(buf);}

    bool hasCompleteRequest()const
    {
        return parser_.state()==HttpParser::State::COMPLETE;
    }

    bool hasError()const
    {
        return parser_.state()==HttpParser::State::ERROR;
    }

    const HttpRequest& request()const{return parser_.request();}

    void reset(){parser_.reset();}

    private:
    HttpParser parser_;
};