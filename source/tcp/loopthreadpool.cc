#include "loopthreadpool.h"
#include <unistd.h>
LoopThreadPool::LoopThreadPool(EventLoop *baseloop)
    : loopthread_num_(0), baseloop_(baseloop), next_idx_(0)
{
}
void LoopThreadPool::SetLoopThreadNum(int n)
{
    loopthread_num_ = n;
}
void LoopThreadPool::Create()
{
    if (loopthread_num_ == 0)
        return;

    loop_threads_.resize(loopthread_num_);
    loops_.resize(loopthread_num_);
    for (int i = 0; i < loopthread_num_; i++)
    {
        loop_threads_[i] = (std::make_unique<LoopThread>());
        loops_[i] = (loop_threads_[i]->GetLoop());
        LOG_DEBUG("loop[%d]: %p  ",i,loop_threads_[i]->GetLoop());
    }
}

EventLoop *LoopThreadPool::NextLoop()
{
    if (loopthread_num_ == 0)
        return baseloop_;
    EventLoop *next = loops_[next_idx_];
    next_idx_ = (next_idx_ + 1) % loopthread_num_;
    LOG_DEBUG("next %d ",next_idx_);
    return next;
};