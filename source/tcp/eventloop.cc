#include "eventloop.h"
// begin eventLoop 管理
EventLoop::EventLoop()
    : thread_id_(std::this_thread::get_id()),
      wakeupfd_(CreatWakeUpFd()), wakeup_channel_(new Channel(this, wakeupfd_)), time_wheel_(this)
{

    wakeup_channel_->SetReadCallBack(std::bind(&EventLoop::ReadWakeFd, this));

    wakeup_channel_->EnAbleRead();
}

void EventLoop::RunAllTasks()
{
    if(tasks_.size() == 0) return;
    std::vector<Functor> tasks;
    {
        std::unique_lock<std::mutex> lock(mtx_);
        std::swap(tasks, tasks_);
    }
    for (auto &task : tasks)
    {
        task();
    }

    return;
}

void EventLoop::RunInLoop(const Functor &cb)
{
    // 判断任务是否在 eventloop 中，如果是则直接执行
    // 否则将任务加入任务队列

    if (IsInLoop())
    {
        return cb();
    }

    return QueueInLoop(cb);
}

void EventLoop::QueueInLoop(const Functor &cb)
{
    // 将任务加入任务队列
    {
        std::unique_lock<std::mutex> lock(mtx_);
        tasks_.push_back(cb);
    }
    // 唤醒 可能阻塞的epoll
    WakeupEventFd();
}

bool EventLoop::IsInLoop()
{
    // 判断当前是否在 eventloop 中
    // 返回 true 或 false
    return thread_id_ == std::this_thread::get_id();
}

void EventLoop::UpDateEvent(Channel *channel)
{
    // 更新事件的代码
    return poller_.UpDateEvent(channel);
}

void EventLoop::ReMoveEvent(Channel *channel)
{
    // 移除事件的代码
    return poller_.ReMoveEvent(channel);
}

void EventLoop::Start()
{
    // 启动事件循环，事件监控 -> 任务就绪事件处理 -> 执行任务
    // 在这个方法中应该会包含事件轮询（比如 Poller），任务队列的处理等
    while (1)
    {
        std::vector<Channel *> actives;
        poller_.Poll(&actives);
        for (auto &channel : actives)
        {
            channel->HandleEvent();
        }
        RunAllTasks();
    }
}
// end;

// begin:定时器模块
void EventLoop::TimerAdd(uint64_t id, uint32_t timeout, const TaskFunc cb)
{
    time_wheel_.timeAdd(id, timeout, cb);
}
void EventLoop::Timerreflash(uint64_t id)
{
    return time_wheel_.reflashTimer(id);
}

void EventLoop::TimerCancel(uint64_t id)
{
    return time_wheel_.timeCancel(id);
}
// end

// begin: 唤醒线程
void EventLoop::WakeupEventFd()
{
    uint64_t wake = 1;
    int n = write(wakeupfd_, &wake, sizeof(wake));
    if (n < 0)
    {
        if (errno == EINTR)
        {
            return;
        }
        LOG_FATAL("eventfd error");
    }
}

void EventLoop::ReadWakeFd()
{
    uint64_t ret = 0;

    int n = read(wakeupfd_, &ret, sizeof(ret));

    if (ret < 0)
    {
        if (errno == EINTR)
        {
            return;
        }
        LOG_FATAL("eventfd error");
    }
    return;
}
int EventLoop::CreatWakeUpFd()
{
    int evfd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (evfd < 0)
    {
        LOG_FATAL("event fd 创建失败");
    }
    LOG_INFO("eventloop wakeup fd success:%d", evfd);
    return evfd;
}
// end;
