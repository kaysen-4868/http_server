#pragma once

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include<string>

class StaticFileHandler
{
    public:

    static const size_t MAX_FILE_SIZE=10*1024*1024;//10MB
    explicit StaticFileHandler(std::string root_dir);

    //根据请求填充响应：文件存在->200 不存在->404 越权->403
    void handle(const HttpRequest& req,HttpResponse& resp);

    private:
    //读文件（二进制），失败返回false
    bool readFile(const std::string&path,std::string& content);

    std::string root_dir_;//不带末尾'/'
};