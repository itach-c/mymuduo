# Simple C++ HTTP Server

这是一个基于 TCP 的高性能 C++ HTTP 服务器，支持静态资源服务与 RESTful 路由处理。

## 🚀 项目特点

- 使用自定义 TCPServer 构建底层网络通信
- 支持 GET / POST / PUT / DELETE 路由注册与匹配
- 支持静态资源请求，设置根目录即可访问 HTML 等文件
- 支持正则路由匹配
- 简洁的 `HttpRequest` / `HttpResponse` 封装
- 支持连接超时销毁与多线程
- http层用了正则匹配后性能显著的下降了，不加正则性能接近每秒4w加了后骤降。

---


