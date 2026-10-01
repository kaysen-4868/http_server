#pragma once

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/StaticFileHandler.h"

inline void handleHttpRequest(const HttpRequest& req,HttpResponse& resp,
                              StaticFileHandler& handler)
{
    if(req.method!="GET"&&req.method!="HEAD")
    {
        resp.status_code=405;
        resp.status_message="Method Not Allowed";
        resp.headers["Content-Type"]="text/plain";
        resp.body="405 Method Not Allowed\n";
        resp.headers["Content-Length"]=std::to_string(resp.body.size());
        return ;
    }
    handler.handle(req,resp);

    
}