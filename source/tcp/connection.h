#pragma once
#include <iostream>
#include "socket.h"
#include "channel.h"
#include "buffer.h"
#include <memory>
#include <any>
class EventLoop;
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    using PtrConnection = std::shared_ptr<Connection>;
    using ConnectionCallBack = std::function<void(const PtrConnection &)>;
    using MessageCallBack = std::function<void(const PtrConnection &, Buffer *)>;
    using CloseCallBack = std::function<void(const PtrConnection &)>;
    using AnyEventCallBack = std::function<void(const PtrConnection &)>;

    using ServerCloseCallBack = std::function<void(const PtrConnection)>;
    typedef enum
    {
        DISCONNECTED,  // 连接关闭状态
        CONNECTING,    // 连接建立成功待处理状态
        CONNECTED,     // 连接建立状态
        DISCONNECTING, // 待关闭状态

    } ConnectStatus;

private:
    uint64_t con_id_; // 连接id
    int sockfd_;      // 连接对应的fd
    Socket socket_;
    ConnectStatus statu_;
    Channel channel_;  // 连接事件管理
    Buffer inbuffer_;  // 接受缓冲区
    Buffer outbuffer_; // 发送缓冲区
    EventLoop *loop_;//这里如果是单线程,那就是baseloop如果是多线程，那主线程只负责accept,从属线程负责监听读写。
    bool enable_inactive_distory_; // 设置为true 会对长时间不通信的 connection销毁默认为false
    uint64_t timerid_;             // 定时器id
    std::any context_;             // 协议上下文

    ConnectionCallBack connected_callback_;
    MessageCallBack message_callback_;
    CloseCallBack close_callback_;
    AnyEventCallBack anyevent_callback_;
    CloseCallBack server_close_callback_; // 组件内的连接关闭回调

private:
    void HandleRead();  // 读时间触发将数据读到 buffer中 然后调用用户的onmessage
    void HandleWrite(); // 将发送缓冲区中的数据发送
    void HandleClose(); // 描述符触发挂断事件在调用用户的 连接挂断回调
    void HandleError(); //
    void HandleEvent(); //
private:
    void SendInLoop(std::string str);

    void ShutDownInLoop(); // 有没有数据待处理有的话发送完再销毁

    void EnableInactiveDistroyInLoop(int second);

    void CancelInactiveDistroyInLoop();

    // 切换协议
    void UpGradeInLoop(std::any context, const ConnectionCallBack &conncb, const CloseCallBack &closecb, const MessageCallBack &msgcb, const AnyEventCallBack &anycb);

    void releseInLoop();
    // 实际的释放
    void EstablishedInLoop(); // 给一个channel设置回调和启动 监控
public:
    Connection(EventLoop *loop, uint64_t conid, int sockfd);
    ~Connection();
    void Send(const char *data, size_t len);
    
    void ShutDown(); // 有没有数据待处理有的话发送完再销毁

    void relese();

    void EnableInactiveDistroy(int second);

    void CancelInactiveDistroy();
    // 切换协议
    void UpGrade(std::any context, const ConnectionCallBack &conncb, const CloseCallBack &closecb, const MessageCallBack &msgcb, const AnyEventCallBack &anycb);

    void Established();

    int GetFd() { return sockfd_; }

    int GetId() { return con_id_; };

    bool Connected() { return statu_ == CONNECTED; }
    // 设置上下文
    void SetContext(std::any context) { context_ = context; }

    std::any *GetContext() { return &context_; } // 获取上下文
    // 下面这四个由服务器模块设置 （其实就是组件使用者设置的）

    void SetConnectedCallBack(const ConnectionCallBack &cb) { connected_callback_ = cb; }
    void SetCloseCallBack(const CloseCallBack &cb) { close_callback_ = cb; }
    void SetMessageCallBack(const MessageCallBack &cb) { message_callback_ = cb; }
    void SetAnyEventCallBack(const AnyEventCallBack &cb) { anyevent_callback_ = cb; } // 组件内的连接关闭回调
    void SetServerCloseCallBack(const ServerCloseCallBack &cb) { server_close_callback_ = cb; }
};