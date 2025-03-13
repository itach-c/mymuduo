// #include "../source/socket.h"
// #include "../source/channel.h"
// #include "../source/eventloop.h"
// #include <ctime>
// void HandleClose(Channel *channel)
// {
//     LOG_INFO("连接关闭");
//     channel->DisAbleAll();
//     channel->ReMoveEvent();
//     std::cout << "before delete" << std::endl;
//     // delete channel;

//     std::cout << "after delete" << std::endl;
// }
// void HandleRead(Channel *channel)
// {
//     std::cout << "读事件触发" << std::endl;

//     char buf[1024];
//     int n = read(channel->GetFd(), buf, sizeof(buf));

//     if (n <= 0)
//     {
//         return HandleClose(channel);
//     }
//     buf[n] = '\0';
//     std::string str(buf);
//     LOG_INFO("%s",str.c_str());
//     channel->EnAbleWrite();
// }
// void HandleWrite(Channel *channel)
// {
//     std::string data = "今天天气不错!";
//     int n = send(channel->GetFd(), data.c_str(), data.size(), 0);
//     if (n < 0)
//     {
//         return HandleClose(channel);
//     }
//     channel->DisAbleWrite();
// }
// void HandleError(Channel *channel)
// {
//     return HandleClose(channel);
// }
// void HandleEvent(Channel *channel, EventLoop *loop, uint64_t id)
// {

//     std::cout << "有一个事件" << std::endl;
//     loop->Timerreflash(id);
// }

// void Accptor(EventLoop *loop, Channel *listen_channel)
// {
//     srand(time(nullptr));
//     uint64_t rand = random() % 100;
//     std::cout << "accptor" << std::endl;

//     int fd = listen_channel->GetFd();

//     int newfd = accept(fd, nullptr, nullptr);

//     Channel *newchannel = new Channel(loop, newfd);

//     newchannel->SetReadCallBack(std::bind(HandleRead, newchannel));
//     newchannel->SetWriteCallBack(std::bind(HandleWrite, newchannel));
//     newchannel->SetErrorCallBack(std::bind(HandleError, newchannel));
//     newchannel->SetCloseCallBack(std::bind(HandleClose, newchannel));
//     newchannel->SetEventsCallBack(std::bind(HandleEvent, newchannel, loop, rand));
//     loop->TimerAdd(rand, 10, std::bind(HandleClose, newchannel));
//     newchannel->EnAbleRead();
// }
// int main()
// {

//     // Poller poller;
//     EventLoop loop;

//     Socket list_sock;
//     list_sock.CreatListenSocket("127.0.0.1", 8888);
//     list_sock.ReuseAddress();
//      LOG_INFO("accept fd success fd: %d",list_sock.Fd());

//     Channel ListenChannel(&loop, list_sock.Fd());

//     ListenChannel.SetReadCallBack(std::bind(Accptor, &loop, &ListenChannel));
//     ListenChannel.EnAbleRead();

//     while (1)
//     {

//         loop.Start();
//     }

//     return 0;
// }

// #include "../source/socket.h"
// #include "../source/channel.h"
// #include "../source/connection.h"
// #include "../source/eventloop.h"
// #include <ctime>
// #include "acceptor.h"
// #include "loopthread.h"
// #include <vector>
// uint64_t conid = 0;
// std::unordered_map<uint64_t, Connection::PtrConnection> connects_;
// std::vector<LoopThread> threads_(2);
// void OnConnection(Connection::PtrConnection conn)
// {
//     std::cout << "get a new connection" << std::endl;
//     LOG_INFO("conn address %p", conn.get());
// }
// void OnMessage(Connection::PtrConnection conn, Buffer *buffer)
// {
//     std::cout << "读事件触发" << std::endl;
//     std::cout << "tid: ---------" << gettid() << std::endl; 

//     std::string str = buffer->ReadAsString(buffer->ReadAbleBytes());
//     std::cout << str.c_str() << std::endl;
//     std::string data = "今天天气不错!";
//     conn->Send(data.c_str(), data.size());
// }
// void OnEvent(Connection::PtrConnection)
// {

//     std::cout << "有一个事件" << std::endl;
//     sleep(1);
// }
// void OnClose(Connection::PtrConnection conn)
// {
//     LOG_INFO("连接关闭");
//     connects_.erase(conn->GetId());
// }
//   EventLoop loop;
//   int nextloop = 0;

// void Accptor(int newfd)
// {
 
//     nextloop = (nextloop+1) % 2 ;
//     std::cout << "获得一个新连接 newfd : " << newfd << std::endl;
//     std::cout << "tid :-----------------------" << gettid() << std::endl;
//     Connection::PtrConnection conn(new Connection(threads_[nextloop].GetLoop(), conid, newfd));
//     connects_.insert(std::make_pair(conid, conn));
//     conid++;

//     conn->SetConnectedCallBack(std::bind(OnConnection, std::placeholders::_1));

//     conn->SetMessageCallBack(std::bind(OnMessage, std::placeholders::_1, std::placeholders::_2));

//     conn->SetAnyEventCallBack(std::bind(OnEvent, std::placeholders::_1));

//     conn->SetCloseCallBack(std::bind(OnClose, std::placeholders::_1));

//     conn->Established();

//     conn->EnableInactiveDistroy(10);
// }
// int main()
// {

//     // Poller poller;
  
//     // Socket list_sock;
//     // list_sock.CreatListenSocket("127.0.0.1", 8888);
//     // list_sock.ReuseAddress();
//     std::bind(OnConnection, std::placeholders::_1);
//     Acceptor acceptor(&loop,"127.0.0.1",8888);
//     acceptor.SetNewConnectionCallback(std::bind(Accptor,std::placeholders::_1));

//    loop.Start();
    

//     return 0;
// }




#include "../../source/tcp/tcpserver.h"

void OnConnection(Connection::PtrConnection conn)
{
    //std::cout << "get a new connection" << std::endl;
   
    LOG_INFO("conn address %p", conn.get());
}
void OnMessage(Connection::PtrConnection conn, Buffer *buffer)
{
   // std::cout << "读事件触发" << std::endl;
    

    std::string str = buffer->ReadAsString(buffer->ReadAbleBytes());
    

    // 构建 HTTP 响应报文
    std::string data = "HTTP/1.1 200 OK\r\n";
    data += "Content-Type: text/html; charset=UTF-8\r\n";
    data += "Content-Length: 41\r\n";
    data += "Connection: close\r\n";  // 表示响应后关闭连接
    data += "\r\n";  // 空行，分隔头部和正文
    data += "<html><body><h1>今天天气不错!</h1></body></html>";

    // 发送响应数据
    conn->Send(data.c_str(), data.size());
    conn ->ShutDown();
 
}

void OnEvent(Connection::PtrConnection)
{

  LOG_INFO("有一个事件");
}
void OnClose(Connection::PtrConnection conn)
{
    LOG_INFO("连接关闭");
   
}
 
int main()
{
    TcpServer server("0.0.0.0",8888,true);
    server.SetThreadNum(3);
    server.SetOnConnectionCallBack(OnConnection);
    server.SetOnMessageCallBack(OnMessage);
    server.SetOnEventsCallBack(OnEvent);
    server.SetOnCloseCallBack(OnClose);
   // server.EnableInactiveDistory(10);
    server.Start();

    return 0;
}