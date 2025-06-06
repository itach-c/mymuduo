#include "poller.h"
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <sys/eventfd.h>
#include "timewheel.h"
class EventLoop
{

    using Functor = std::function<void()>;

public:
    void RunAllTasks();

public:
    EventLoop();

    static int CreatWakeUpFd();

    void RunInLoop(const Functor &cb); // 判断任务是否在eventloop中 是则执行 不是则 压入池中

    void QueueInLoop(const Functor &cb);

    bool IsInLoop();

    void UpDateEvent(Channel *channel);

    void ReMoveEvent(Channel *channel);

    void Start(); // 事件监控 -》 任务就绪事件处理 -》 执行任务

    void ReadWakeFd();
    // 唤醒eventloop线程
    void WakeupEventFd();
    // 添加定时任务
    void TimerAdd(uint64_t id, uint32_t timeout, const TaskFunc cb);

    void Timerreflash(uint64_t id);

    void TimerCancel(uint64_t id);

    bool HasTimer(uint64_t id) { return time_wheel_.Hastimer(id); }

    void AssertInLoop() { assert(thread_id_ == std::this_thread::get_id()); }
    std::thread::id getId(){return thread_id_;}
    
private:
    std::thread::id thread_id_;
    int wakeupfd_; // 唤醒线程使用
    std::unique_ptr<Channel> wakeup_channel_;

    Poller poller_;
    std::vector<Functor> tasks_;
    std::mutex mtx_;

    TimeWheel time_wheel_;
};