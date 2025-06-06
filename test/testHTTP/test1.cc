#include "httpServer.h"
#include <string>
#include <sstream>

std::string strRequest(const HttpRequest &req)
{
    std::ostringstream resp;

    // 拼接请求行
    resp << req.method_ << " " << req.request_path_;

    // 如果有查询参数，拼接查询字符串
    if (!req.search_params_.empty())
    {
        resp << "?";
        for (auto it = req.search_params_.begin(); it != req.search_params_.end(); ++it)
        {
            if (it != req.search_params_.begin())
            {
                resp << "&";
            }
            resp << it->first << "=" << it->second;
        }
    }

    resp << " " << req.version_ << "\r\n";

    // 拼接头部字段
    for (const auto &head : req.headers_)
    {
        resp << head.first << ": " << head.second << "\r\n";
    }

    // 头部结束后添加一个空行
    resp << "\r\n";
 //std::cout << resp.str() << std::endl;
    return resp.str();
}

void hello(const HttpRequest &req, HttpResPonse *resp)
{
    std::string str = strRequest(req);
    //std::cout << str << std::endl;
    resp->SetContent(str,"text/plain");

}
void putfile(const HttpRequest &req, HttpResPonse *resp)
{
  
    std::string path = "./wwwroot" + req.request_path_;
    std::cout << "path: " << path << std::endl;
    util::WriteFile(path,req.content_);
}
void post(const HttpRequest &req, HttpResPonse *resp)
{
    std::string str = strRequest(req);
    resp->SetContent(str);
}
void del(const HttpRequest &req, HttpResPonse *resp)
{
    std::string str = strRequest(req);
    resp->SetContent(str);
}
int main()
{
    HttpServer server("0.0.0.0", 8888, true);
    //server.EnableInactiveDestroy(5);
    server.SetBaseDir("./wwwroot");

    server.SetThreadNum(4);
    server.Get("/hello", std::move(hello));
    server.Put("/putfile", std::move(putfile));
    server.Post("/post", std::move(post));
    server.Delete("/del", std::move(del));
   
    server.Listen();

    return 0;
}
