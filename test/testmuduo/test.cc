#include "muduo/net/http/HttpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/http/HttpRequest.h"
#include "muduo/net/http/HttpResponse.h"
#include "muduo/net/http/HttpContext.h"
#include <iostream>
#include <sstream>
#include <string>
#include "muduo/base/Logging.h"
using namespace muduo::net;
std::string strRequest(const HttpRequest &req)
{

  
    std::ostringstream resp;

    // 拼接请求行
    resp << req.method() << " " << req.path();

    resp << " " << req.query() << "\r\n";

    // 拼接头部字段
    for (const auto &head : req.headers())
    {
        resp << head.first << ": " << head.second << "\r\n";
    }

    // 头部结束后添加一个空行
    resp << "\r\n";
    return resp.str();
}

void hello(const HttpRequest &req, HttpResponse *resp)
{

    if(req.path() == "/hello")
    {
        std::string str = strRequest(req);
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setBody(str);
        resp->setContentType("text/plain");
    }
}
int main()
{


    EventLoop loop;
    muduo::Logger::setLogLevel(muduo::Logger::LogLevel::FATAL);
    HttpServer Server(&loop, InetAddress("0.0.0.0", 8888), "http", TcpServer::kReusePort);
    Server.setThreadNum(4);
    Server.setHttpCallback(hello);
    Server.start();
    loop.loop();
    
    return 0;
}