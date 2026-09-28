#pragma once

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

inline void handleHttpRequest(const HttpRequest& req,HttpResponse& resp)
{
    if(req.method!="GET")
    {
        resp.status_code=405;
        resp.status_message="Method Not Allowed";
        resp.headers["Content-Type"]="text/plain";
        resp.body="405 Method Not Allowed\n";
        return ;
    }

    if(req.path=="/")
    {
        resp.status_code=200;
        resp.status_message="OK";
        resp.headers["Content-Type"]="text/html";
        resp.body="<h1>Hello,HTTP!<h1>\n";
    }

    resp.status_code=404;
    resp.status_message="Not Found";
    resp.headers["Content-Type"]="text/plain";
    resp.body="404 Not Found\n";
}