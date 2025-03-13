#include "httpServer.h"

HttpServer::HttpServer(const std::string &ip, uint32_t port, bool Portreuse)
    : tcpserver_(ip, port, Portreuse)
{
    // 构造函数实现
    tcpserver_.SetOnConnectionCallBack(std::bind(&HttpServer::OnConnection, this, std::placeholders::_1));
    tcpserver_.SetOnMessageCallBack(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void HttpServer::SetBaseDir(const std::string &path)
{
    basedir_ = path;
    // 设置静态资源目录
}

void HttpServer::Get(const std::string &pattern, Handler handler)
{
    route_get_.push_back(std::make_pair(std::regex(pattern), handler));
    // 处理GET请求的路由注册
}

void HttpServer::Post(const std::string &pattern, Handler handler)
{
    route_post_.push_back(std::make_pair(std::regex(pattern), handler));
    // 处理POST请求的路由注册
}

void HttpServer::Put(const std::string &pattern, Handler handler)
{
    route_put_.push_back(std::make_pair(std::regex(pattern), handler));
    // 处理PUT请求的路由注册
}

void HttpServer::Delete(const std::string &pattern, Handler handler)
{
    route_delete_.push_back(std::make_pair(std::regex(pattern), handler));
    // 处理DELETE请求的路由注册
}

void HttpServer::SetThreadNum(int num)
{
    tcpserver_.SetThreadNum(num);
    // 设置线程数
}

void HttpServer::EnableInactiveDestroy(int timeout)
{
    tcpserver_.EnableInactiveDistory(timeout);
    // 启用连接空闲超时销毁
}

void HttpServer::OnConnection(Connection::PtrConnection conn)
{
    // 设置连接的上下文

    LOG_INFO("get a new connection %p", conn.get());
    conn->SetContext(HttpContext());
}
void HttpServer ::HandleError(HttpResPonse *resp)
{

    std::string body;
    body += "<html>";
    body += "<head>";
    body += "<meta http-equiv='Content-Type' content='text/html;charset=utf-8'>";
    body += "</head>";
    body += "<body>";
    body += "<h1>";
    body += std::to_string(resp->GetStatusCode());
    body += " ";
    body += util::getStatusMessage(resp->GetStatusCode());
    body += "</h1>";
    body += "</body>";
    body += "</html>";
    resp->SetContent(body);
}

void HttpServer ::Handle404(HttpResPonse *resp)
{
    resp->SetStatueCode(404);
    std::string html =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>404 Not Found</title>\n"
        "    <style>\n"
        "        body {\n"
        "            font-family: Arial, sans-serif;\n"
        "            background-color: #f4f4f4;\n"
        "            text-align: center;\n"
        "            color: #333;\n"
        "        }\n"
        "        h1 {\n"
        "            color: #ff0000;\n"
        "            font-size: 72px;\n"
        "        }\n"
        "        p {\n"
        "            font-size: 24px;\n"
        "        }\n"
        "        .container {\n"
        "            max-width: 600px;\n"
        "            margin: 0 auto;\n"
        "            padding: 50px;\n"
        "            background-color: white;\n"
        "            border-radius: 8px;\n"
        "            box-shadow: 0 0 15px rgba(0, 0, 0, 0.1);\n"
        "        }\n"
        "        .home-link {\n"
        "            margin-top: 30px;\n"
        "            padding: 10px 20px;\n"
        "            background-color: #4CAF50;\n"
        "            color: white;\n"
        "            text-decoration: none;\n"
        "            border-radius: 5px;\n"
        "        }\n"
        "        .home-link:hover {\n"
        "            background-color: #45a049;\n"
        "        }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <div class=\"container\">\n"
        "        <h1>404</h1>\n"
        "        <p>Sorry, the page you are looking for does not exist.</p>\n"
        "        <a href=\"/\" class=\"home-link\">Go to Home</a>\n"
        "    </div>\n"
        "</body>\n"
        "</html>";
        resp->SetContent(std::move(html));
}
void HttpServer::OnMessage(Connection::PtrConnection conn, Buffer *buffer)
{

    while (buffer->ReadAbleBytes() > 0)
    {

        // HttpContext context = std::any_cast<HttpContext>(*conn->GetContext());

        HttpContext *context = std::any_cast<HttpContext>(conn->GetContext());
        bool ret = context->Rcv_Context_HttpReq(buffer); // 尝试拿到一个http报文但是有可能是半个 一个 或者一个半...或者是一个错误的报文
        HttpRequest req = context->GetHttp();
        HttpResPonse resp(context->GetRespStatuCode());
        resp.SetVersion(req.version_);
        if (context->GetRcvStatue() == HttpContext::RCV_ERROR)
        {
            // 报文错了直接关闭连接并清空缓冲区,给客户端响应一个错误信息和错误状态码
            HandleError(&resp); // handle Error中把错误直接发回去

            BuildResponseAndSend(conn, req, resp);
            buffer->Clear();
            conn->relese();
            return;
        }
        else // 报文格式没错,但是得分类
        {
            if (context->GetRcvStatue() == HttpContext::RCV_OVER)
            {
                std::cout << req.request_path_ << std::endl;
                if (util::isValidPath(req.request_path_) == false)
                {
                    Handle404(&resp);
                    BuildResponseAndSend(conn, req, resp);
                    conn->ShutDown();
                }

                Route(req, &resp); // 根据请求方法去进行一下路由
                BuildResponseAndSend(conn, req, resp);
                context->Reset();
            }
            else
            {
                return; // 其余情况暂时先不关闭连接，如果后续数据到达接着组装解析，如果超过非活跃连接断开，那自动关闭
            }
           // if (req.isShortConnection() == true)
                conn->ShutDown();
        }
    }
}

void HttpServer::Route(HttpRequest &req, HttpResPonse *resp)
{
    // 路由请求处理 判断请求的是静态资源 还是功能路由

    if (req.method_ == "GET" || req.method_ == "HEAD")
    {
        bool ret = isStaticRequest(req, resp);
        if (ret == true)
        {
            HandleStaticSource(req, resp);
            return;
        }
    }
    // 功能路由
    if (req.method_ == "GET" || req.method_ == "HEAD")
    {
        return Dispatcher(req, resp, route_get_);
    }
    else if (req.method_ == "POST")
    {
        return Dispatcher(req, resp, route_post_);
    }
    else if (req.method_ == "PUT")
    {
        return Dispatcher(req, resp, route_put_);
    }
    else if (req.method_ == "DELETE")
    {
        return Dispatcher(req, resp, route_delete_);
    }
    else
    {
        return resp->SetStatueCode(405);
    }
}
bool HttpServer::isStaticRequest(HttpRequest &req, HttpResPonse *resp)
{ // 处理静态资源

    if (req.method_ != "GET" && req.method_ != "POST")
        return false;

    if (req.request_path_ == "/")
    {
        if (basedir_.empty()) //
        {
            LOG_FATAL("必须设置根目录路径");
        }
        req.request_path_ = basedir_ + req.request_path_;
        req.request_path_ += "index.html";
        return true;
    }
    if (util::isRegular(req.request_path_) == false)
    {
        return false;
    }
    req.request_path_ = basedir_ + req.request_path_;
    return true;
}
bool HttpServer::HandleStaticSource(HttpRequest &req, HttpResPonse *resp)
{
    // 将静态资源内容读取出来放到 resp body中
    std::string content;
    bool ret = util::ReadFile(req.request_path_, &content);

    if (ret == false)
        return false;
    std::string mime = util::GetMime(req.request_path_);
    ;
    resp->SetContent(std::move(content), mime);

    return true;
}

void HttpServer::Dispatcher(HttpRequest &req, HttpResPonse *resp, HandlerRoute &route)
{
    // 分发请求处理
    for (auto &r : route)
    {
        std::regex re(r.first);
        bool ret = std::regex_match(req.request_path_, req.match_, re);
        if (ret == false)
        {
            continue;
        }
        else
        {
            return r.second(req, resp);
        }
    }
    // 没找到这个功能路由 也不是静态资源请求
   return Handle404(resp);
}

void HttpServer::BuildResponseAndSend(Connection::PtrConnection conn, HttpRequest &req, HttpResPonse &resp)
{
    // 构建HTTP响应
    std::string httpResLine = resp.GetVersion() + " " + std::to_string(resp.GetStatusCode()) + " " + util::getStatusMessage(resp.GetStatusCode()) + "\r\n"; // 请求行

    std::string httpHeaders;
    for (auto &h : resp.GetHeaders())
    {
        httpHeaders += h.first;
        httpHeaders += ": ";
        httpHeaders += h.second;
        httpHeaders += "\r\n";
    }
    httpHeaders += "\r\n";
    std::string httpResp = httpResLine + httpHeaders + resp.GetContent();
    conn->Send(httpResp.c_str(), httpResp.size());
}

void HttpServer::Listen()
{
    tcpserver_.Start();
}