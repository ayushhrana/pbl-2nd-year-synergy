#pragma once

#include <string>

namespace expense_tracker
{

struct HttpRequest
{
    std::string method;
    std::string path;
    std::string body;
};

struct HttpResponse
{
    int status = 200;
    std::string contentType = "text/plain";
    std::string body;
};

class RequestHandler
{
public:
    virtual ~RequestHandler()
    {
    }

    virtual HttpResponse handleRequest(const HttpRequest& request) = 0;
};

class HttpServer
{
public:
    HttpServer(RequestHandler& handler) : handler(handler)
    {
    }

    void run(int port);

private:
    RequestHandler& handler;
};

}
