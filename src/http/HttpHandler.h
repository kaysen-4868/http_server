#pragma once

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/StaticFileHandler.h"

inline void handleHttpRequest(const HttpRequest &req, HttpResponse &resp,
                              StaticFileHandler &handler) {
  if (req.method == "GET" || req.method == "HEAD") {
    handler.handle(req, resp);

    // HEAD不返回body,但要保留content-length
    if (req.method == "HEAD") {
      resp.body.clear();
    }
    return;
  }

  if (req.method == "POST" || req.method == "PUT" || req.method == "DELETE") {
    resp.status_code = 501;
    resp.status_message = "Not Implemented";
    resp.headers["Content-Type"] = "text/plain";
    resp.body = "501 Not Implemented\n";
    resp.headers["Content-Length"] = std::to_string(resp.body.size());
    return;
  }

  resp.status_code = 405;
  resp.status_message = "Method Not Allowed";
  resp.headers["Content-Type"] = "text/plain";
  resp.body = "405 Method Not Allowed\n";
  resp.headers["Content-Length"] = std::to_string(resp.body.size());
}