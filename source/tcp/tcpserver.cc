#include "tcpserver.h"
static NetWork network;
TcpServer::TcpServer(const std::string &ip, uint32_t port,bool portReuse)
    : loopthread_pool_(&baseloop_), acceptor_(&baseloop_, ip, port,portReuse), nextid_(0),
      enable_inactive_distory_(false), threadnum_(0)
{

    acceptor_.SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));

}
TcpServer::~TcpServer()
{
}

void TcpServer::Start()
{
    loopthread_pool_.SetLoopThreadNum(threadnum_);
    loopthread_pool_.Create();
    return baseloop_.Start();
}
void TcpServer::RunAfter(const Functor &task, int delay)
{
    baseloop_.RunInLoop(std::bind(&TcpServer::RunAfterInLoop, this, task, delay));
}
void TcpServer::RunAfterInLoop(const Functor &task, int delay)
{
    nextid_++;
    baseloop_.TimerAdd(nextid_, delay, task);
}

void TcpServer::ReMoveConnection(const Connection::PtrConnection &conn)
{
    baseloop_.RunInLoop(std::bind(&TcpServer::ReMoveConnectionInLoop, this, conn));
}

void TcpServer::ReMoveConnectionInLoop(const Connection::PtrConnection &conn)
{
    int id = conn->GetId();

    auto it = connections_.find(id);

    if (it != connections_.end())
        connections_.erase(id);
}

void TcpServer::NewConnection(int fd)
{
    //std::cout << "连接处理线程 tid: " << gettid() << std::endl;
    
    EventLoop* nextloop = loopthread_pool_.NextLoop();
    LOG_DEBUG("newfd %d -> eventloop %p ",fd ,nextloop);
    Connection::PtrConnection conn(new Connection(nextloop, nextid_, fd));
    connections_.insert(std::make_pair(nextid_, conn));
    nextid_++;
  

    conn->SetConnectedCallBack(connect_callback_);

    conn->SetMessageCallBack(message_callback_);

    conn->SetAnyEventCallBack(event_callback_);

    conn->SetCloseCallBack(close_callback_);

    conn->SetServerCloseCallBack(std::bind(&TcpServer::ReMoveConnection, this, std::placeholders::_1));

    conn->Established();

    if (enable_inactive_distory_ == true)
        conn->EnableInactiveDistroy(timeout_);
}
