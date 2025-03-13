#include "poller.h"
Poller::Poller()
{

    int epfd = epoll_create(1024);
    if (epfd < 0)
    {

        LOG_FATAL("epfd创建失败程序终止");
    }
    epollfd_ = epfd;
}

void Poller::UpDateEvent(Channel *channel)
{
    if (HasChannel(channel))
    {
        // 存在则更新
        Update(channel, EPOLL_CTL_MOD);
    }

    else
    { // 不存在则添加

        channels_.insert(std::make_pair(channel->GetFd(), channel));
        Update(channel, EPOLL_CTL_ADD);
    }
}

// 移除监控
void Poller::ReMoveEvent(Channel *channel)
{
    auto it = channels_.find(channel->GetFd());
    if (it != channels_.end())
    {
        channels_.erase(channel->GetFd());
        Update(channel, EPOLL_CTL_DEL);
    }
}

// 开始监控返回活跃连接
void Poller::Poll(std::vector<Channel *> *active)
{
    errno = 0;
    // int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout);

    int ReadyNum = epoll_wait(epollfd_, evs_, MAX_EPOLLEVENTS, -1);
    if (ReadyNum < 0)
    {
        if (errno == EINTR)
        {
            LOG_ERROR("epoll_wait 被信号打断");
            return;
        }

        LOG_FATAL("epoll wait Error");
    }
    for (int i = 0; i < ReadyNum; i++)
    {
        int Readyfd = evs_[i].data.fd;
        uint32_t ReadyEvents = evs_[i].events;

        auto it = channels_.find(Readyfd);

        it->second->SetRevents(ReadyEvents);

        assert(it != channels_.end());

        (*active).push_back(it->second);
    }
}

// 对epoll操作
void Poller::Update(Channel *channel, int op)
{

    struct epoll_event ev;

    ev.events = channel->Events();
    ev.data.fd = channel->GetFd();
    int n = epoll_ctl(epollfd_, op, channel->GetFd(), &ev);
    if (n < 0)
    {
        LOG_ERROR("epoll ctl error errno :%d ,error message %s", errno, strerror(errno));
    }
}

bool Poller::HasChannel(Channel *channel)
{

    auto it = channels_.find(channel->GetFd());

    return !(it == channels_.end());
}
