#include "loopthread.h"
#include "eventloop.h"
LoopThread::LoopThread()
    : loop_(nullptr), loopthread_(std::thread(&LoopThread::ThreadEntry, this))
{
}

EventLoop *LoopThread::GetLoop()
{
    std::unique_lock<std::mutex> lock(mtx_);
    con_.wait(lock, [this]() -> bool
              { return loop_ != nullptr; });
    return loop_;
}
void LoopThread::ThreadEntry()
{
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mtx_);

        loop_ = &loop;
    }
    con_.notify_one();
    loop_->Start();
}
