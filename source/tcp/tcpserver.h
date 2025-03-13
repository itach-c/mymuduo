#pragma once
#include <iostream>
#include "loopthreadpool.h"
#include <unordered_map>
#include "connection.h"
#include "eventloop.h"
#include <functional>
#include "buffer.h"
#include "acceptor.h"
#include <signal.h>
#include "../comm/log.hpp"
using Functor = std::function<void()>;
class TcpServer
{
    using MessageCallBack = std::function<void(Connection::PtrConnection, Buffer *buffer)>;
    using ConnectionCallBack = std::function<void(Connection::PtrConnection)>;
    using CloseCallBack = std::function<void(Connection::PtrConnection)>;
    using EventCallBack = std::function<void(Connection::PtrConnection)>;

public:
    TcpServer(const std::string &ip, uint32_t port,bool portReuse = false);
    ~TcpServer();
    void SetThreadNum(int num) { threadnum_ = num;};
    void Start();

    void EnableInactiveDistory(int timeout){ timeout_ = timeout;
    enable_inactive_distory_ = true; }

    void SetOnMessageCallBack(const MessageCallBack &cb) { message_callback_ = cb; }

    void SetOnConnectionCallBack(const ConnectionCallBack &cb) { connect_callback_ = cb; }

    void SetOnEventsCallBack(const EventCallBack &cb) {event_callback_ = cb;}

    void SetOnCloseCallBack(const CloseCallBack &cb) { close_callback_ = cb; }

    void RunAfter(const Functor &task, int delay); // 添加一个定时任务

public:
private:

    void NewConnection(int fd);

    void ReMoveConnection(const Connection::PtrConnection& conn);

    void ReMoveConnectionInLoop(const Connection::PtrConnection& conn);

    void RunAfterInLoop(const Functor &task, int delay);

   

private:
    EventLoop baseloop_; // 主线程中运行的 eventloop

    LoopThreadPool loopthread_pool_; // 从属reactor的线程池

    std::unordered_map<uint64_t, Connection::PtrConnection> connections_; // 服务器管理的所有连接

    MessageCallBack message_callback_; // 消息到来回调

    ConnectionCallBack connect_callback_; // 连接建立回调

    CloseCallBack close_callback_; // 连接关闭回调

     EventCallBack event_callback_;


    Acceptor acceptor_; // 连接管理器只负责监听连接

    uint64_t nextid_;

    int threadnum_; // 从属 reactor 个数

    int timeout_; // 设置非活跃连接超时时间;

    bool enable_inactive_distory_; // 启动非活跃销毁
};


class NetWork
{
    public:
    NetWork()
    {
        LOG_DEBUG("SIGPIPE INT");
        signal( SIGPIPE,SIG_IGN);
    }

};
