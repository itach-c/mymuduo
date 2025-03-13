#include "connection.h"
#include "eventloop.h"
Connection::Connection(EventLoop *loop, uint64_t conid, int sockfd)
    : loop_(loop), con_id_(conid), timerid_(conid), sockfd_(sockfd), statu_(CONNECTING), socket_(sockfd_), channel_(loop, sockfd_), enable_inactive_distory_(false)
{
    channel_.SetCloseCallBack(std::bind(&Connection::HandleClose, this));
    channel_.SetErrorCallBack(std::bind(&Connection::HandleError, this));
    channel_.SetReadCallBack(std::bind(&Connection::HandleRead, this));
    channel_.SetEventsCallBack(std::bind(&Connection::HandleEvent, this));
    channel_.SetWriteCallBack(std::bind(&Connection::HandleWrite, this));
}

Connection::~Connection()
{
    // 析构函数清理资源
    LOG_INFO("connection release: %p", this);
}

// 读事件触发，将数据读到 buffer 中，然后调用用户的 onmessage
void Connection::HandleRead()
{
    // 1 .接受socket数据放到缓冲区
    // 2.调用 onmessage;
    char buf[65535];
    int n = socket_.NonBlockRecv(buf, sizeof(buf));
    if (n < 0)
    {
        if (errno == EAGAIN || EINTR)
            return;
        else
            return ShutDown();
    }
    else if (n == 0)
    {
        return;
    }
    inbuffer_.Write(buf, n);
    if (inbuffer_.ReadAbleBytes() > 0)
    {

        return message_callback_(shared_from_this(), &inbuffer_);
    }
}

// 写事件触发，将发送缓冲区中的数据发送
void Connection::HandleWrite()
{
    //

    ssize_t n = socket_.NonBlockSend(outbuffer_.ReadPosition(), outbuffer_.ReadAbleBytes());
    if (n < 0)
    {
        // 发送错误要关闭连接了
        if (inbuffer_.ReadAbleBytes() > 0)
        {
            message_callback_(shared_from_this(), &inbuffer_);
        }
        return relese();
    }
    outbuffer_.MoveReadOffset(n);

    if (outbuffer_.ReadAbleBytes() == 0)
    {
        channel_.DisAbleWrite(); // 没有数据待发送 关闭写
        if (statu_ == DISCONNECTING)
        {
            return relese();
        }
    }
    return;
}

// 描述符触发挂断事件
void Connection::HandleClose()
{
    // 连接挂断，如果有数据待处理,那就处理,处理完成关闭连接
    if (inbuffer_.ReadAbleBytes() > 0)
    {
        message_callback_(shared_from_this(), &inbuffer_);
    }
    return relese();
}

// 错误事件触发处理
void Connection::HandleError()
{

    if (inbuffer_.ReadAbleBytes() > 0)
    {
        message_callback_(shared_from_this(), &inbuffer_);
    }
    return relese();
}

// 事件触发处理的主函数
void Connection::HandleEvent()
{
    // 延迟定时销毁任务
    // 调用 组件使用者的任务
    if (enable_inactive_distory_ == true)
    {
        loop_->Timerreflash(timerid_);
    }
    if (anyevent_callback_)
        anyevent_callback_(shared_from_this());
}

// 如果有数据待处理，发送完再销毁连接
void Connection::ShutDownInLoop()
{
    if (statu_ == DISCONNECTING)
        return;
    statu_ = DISCONNECTING;
    if (inbuffer_.ReadAbleBytes() > 0)
    {
        if (message_callback_)
            message_callback_(shared_from_this(), &inbuffer_);
    }
    if (outbuffer_.ReadAbleBytes() > 0)
    {
        if (channel_.WriteAble() == false)
        {
            channel_.EnAbleWrite();
        }
    }
    if (outbuffer_.ReadAbleBytes() == 0)
    {
        relese();
    }
}
// 关闭连接，如果有数据待处理，发送完再销毁连接
void Connection::ShutDown()
{
    if (loop_->IsInLoop())
    {
        ShutDownInLoop();
    }
    else
    {
        loop_->RunInLoop(std::bind(&Connection::ShutDownInLoop, this));
    }
}

// 取消在指定时间内销毁连接
void Connection::CancelInactiveDistroyInLoop()
{
    //
    enable_inactive_distory_ = true;
    if (loop_->HasTimer(timerid_))
    {
        loop_->TimerCancel(timerid_);
    }
}

// 取消在指定时间内销毁连接
void Connection::CancelInactiveDistroy()
{
    // 函数体: 实现取消销毁连接的逻辑
    if (loop_->IsInLoop())
    {
        CancelInactiveDistroyInLoop();
    }
    else
    {
        loop_->RunInLoop(std::bind(&Connection::CancelInactiveDistroyInLoop, this));
    }
}
// 切换协议并更新回调
void Connection::UpGrade(std::any context, const ConnectionCallBack &conncb, const CloseCallBack &closecb,
                         const MessageCallBack &msgcb, const AnyEventCallBack &anycb)
{
    // 函数体: 实现切换协议的逻辑
    // 这个函数必须在EventLoop中立即执行,如果在其他线程中调用
    //,会将操作压入队列,此时没修改协议，可能会造成新来的数据用旧的协议处理
    loop_->AssertInLoop();
    loop_->RunInLoop(std::bind(&Connection::UpGradeInLoop, this, context, conncb, closecb, msgcb, anycb));
}
// 切换协议，更新回调
void Connection::UpGradeInLoop(std::any context, const ConnectionCallBack &conncb, const CloseCallBack &closecb,
                               const MessageCallBack &msgcb, const AnyEventCallBack &anycb)
{
    //// 切换协议并更新回调的实现
    context_ = context;
    connected_callback_ = conncb;
    close_callback_ = closecb;
    message_callback_ = msgcb;
    anyevent_callback_ = anycb;
}

// 实际的资源释放
void Connection::releseInLoop()
{
    // 修改连接状态,将其置为disconnectioned

    statu_ = DISCONNECTED;
    // 移除事件监控 关闭描述符

    channel_.ReMoveEvent();
    socket_.Close();
    // 定时器队列中还有定时任务 则销毁它
    if (loop_->HasTimer(con_id_))
    {
        loop_->TimerCancel(con_id_);
    }
    // 通过回调告知用户连接已经关闭
    if (close_callback_)
        close_callback_(shared_from_this());

    // 告知服务器,服务器移除管理信息
    if (server_close_callback_)
        server_close_callback_(shared_from_this());
}

void Connection::relese()
{
    if (statu_ = DISCONNECTED)
        return;

    return loop_->QueueInLoop(std::bind(&Connection::releseInLoop, this));
}

// 给一个 channel 设置回调并启动监控
void Connection::EstablishedInLoop()
{

    // 修改连接状态,启动监控，调用回调
    assert(statu_ == CONNECTING);
    statu_ = CONNECTED;
    channel_.EnAbleRead();
    if (connected_callback_)
        connected_callback_(shared_from_this());
}

// 设置连接已建立状态
void Connection::Established()
{
    // 函数体: 实现设置连接已建立状态的逻辑
    if (loop_->IsInLoop())
    {
        EstablishedInLoop();
    }
    else
    {

        loop_->RunInLoop(std::bind(&Connection::EstablishedInLoop, this));
    }
}

// 发送数据（在事件循环中）
void Connection::SendInLoop(std::string str)
{
    // 实现代码...

    if (statu_ == DISCONNECTED)
        return;
    outbuffer_.Write(str.c_str(), str.size());
    if (channel_.WriteAble() == false)
    {
        channel_.EnAbleWrite();
    }
}

// 发送数据
void Connection::Send(const char *data, size_t len)
{
    if (loop_->IsInLoop())
    {
        SendInLoop(std::string(data, data + len));
    }
    else
    {
        loop_->RunInLoop(std::bind(&Connection::SendInLoop, this, std::string(data, data + len)));
    }
}
// 启用在指定时间内销毁连接
void Connection::EnableInactiveDistroyInLoop(int second)
{
    enable_inactive_distory_ = true;
    if (loop_->HasTimer(timerid_))
    {
        loop_->Timerreflash(timerid_);
    }
    else
    {
        loop_->TimerAdd(timerid_, second, std::bind(&Connection::relese, this));
    }
}

// 启用在指定时间内销毁连接
void Connection::EnableInactiveDistroy(int second)
{
    // 函数体: 实现启用销毁连接的逻辑
    if (loop_->IsInLoop())
    {
        EnableInactiveDistroyInLoop(second);
    }
    else
    {
        loop_->RunInLoop(std::bind(&Connection::EnableInactiveDistroyInLoop, this, second));
    }
}
