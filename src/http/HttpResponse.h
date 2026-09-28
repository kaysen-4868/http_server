#pragma once

#include<string>
#include<unordered_map>

class HttpResponse
{
    public:
    int status_code =200;
    std::string status_message="OK";
    std::unordered_map<std::string,std::string>headers;
    std::string body;

    std::string toString()const
    {
        std::string result;
        result+="HTTP/1.1"+std::to_string(status_code)
        +" "+status_message+"\r\n";

        for(const auto& [k,v]:headers)
        result+=k+":"+v+"\r\n";

        result+="\r\n";
        result+=body;
        return result;
    }

    void reset()
    {
        status_code=200;
        status_message="OK";
        headers.clear();
        body.clear();
    }
};