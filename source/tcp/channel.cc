
#include "channel.h"
#include "eventloop.h"
Channel::Channel(EventLoop *eventloop, int fd) : fd_(fd), events_(0), revents_(0), eventloop_(eventloop) {}
Channel::~Channel() {};

int Channel::GetFd() { return fd_; }

void Channel::SetReadCallBack(const EventCallBack &cb) { ReadCallBack_ = cb; }

void Channel::SetWriteCallBack(const EventCallBack &cb) { WriteCallBack_ = cb; }

void Channel::SetErrorCallBack(const EventCallBack &cb) { ErrorCallBack_ = cb; }

void Channel::SetCloseCallBack(const EventCallBack &cb) { CloseCallBack_ = cb; }

void Channel::SetEventsCallBack(const EventCallBack &cb) { EventsCallBack_ = cb; }

bool Channel::ReadAble() { return events_ & EPOLLIN; }

bool Channel::WriteAble() { return events_ & EPOLLOUT; }
// 下面这几个后序会通过eventloop在poller中添加
void Channel::EnAbleRead()
{
    events_ |= EPOLLIN;
    UpDateEvent();
} // for test

void Channel::EnAbleWrite()
{
    events_ |= EPOLLOUT;
    UpDateEvent();
}

void Channel ::DisAbleRead()
{
    events_ &= ~EPOLLIN;
    UpDateEvent();
}

void Channel::DisAbleWrite()
{
    events_ &= ~EPOLLOUT;
    UpDateEvent();
}

void Channel::DisAbleAll()
{
    events_ = 0;
    UpDateEvent();
}
//
void Channel::SetRevents(uint32_t events) { revents_ = events; }

uint32_t Channel::Events() { return events_; }

// for test
void Channel::UpDateEvent()
{
    eventloop_->UpDateEvent(this);
}
void Channel::ReMoveEvent()
{

    eventloop_->ReMoveEvent(this);
}

void Channel::HandleEvent()
{

    // 触发可读事件,连接挂断,优先数据则调用读回调
    if (revents_ & EPOLLIN || revents_ & EPOLLHUP || revents_ & EPOLLPRI)
    {

       // printf("fd %d 读事件触发 准备调用读回调 tid: %u\n",fd_,gettid());
        if (ReadCallBack_)
        {
            ReadCallBack_();
        }
    }

    // 对于可能触发连接关闭的回调只处理一个,例如写时发现连接关闭,则关闭套接字
    if (revents_ & EPOLLOUT)
    {
       // printf("fd %d 写事件触发 准备调用读回调tid: %u\n",fd_,gettid());

        if (WriteCallBack_)
        {
            WriteCallBack_();
        }
    }

    else if (revents_ & EPOLLERR)
    {

        if (ErrorCallBack_)
        {
            ErrorCallBack_();
        }
    }
    else if (revents_ & EPOLLHUP)
    {

        if (CloseCallBack_)
        {
            CloseCallBack_();
        }
    }

    if (EventsCallBack_)
    {
        EventsCallBack_();
    }
}
