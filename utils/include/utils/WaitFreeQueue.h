#pragma once

#include <atomic>
#include <deque>

struct SpinLock
{
    std::atomic<bool> lock_{false};
    FORCE_INLINE void unlock()
    {
        std::atomic_store_explicit(&lock_, false, std::memory_order_release);
    }
    FORCE_INLINE void lock()
    {
        while(std::atomic_exchange_explicit(&lock_, true, std::memory_order_acquire))
             ; // spin until acquired
    }
};

template<typename T>
class WaitFreeQueue
{
public:
    
    WaitFreeQueue() = default;
    ~WaitFreeQueue() = default;
    WaitFreeQueue(const WaitFreeQueue&) = delete;
    WaitFreeQueue& operator=(const WaitFreeQueue&) = delete;
    
    void push_back(T&& data)
    {
        lock_.lock();
        datas_.emplace_back(std::forward<T>(data));
        lock_.unlock();
    }
    
    auto&& pop_front()
    {
        static T nodata;
        do
        {
            lock_.lock();
            if (!datas_.empty()) break;
            lock_.unlock();
//            std::this_thread::yield();
            if (unlikely(dontSpin_)) return std::move(nodata);
        } while(1);
        T&& data = std::move(datas_.front());
        datas_.pop_front();
        lock_.unlock();
        return std::forward<T>(data);
    }
    
    void dontSpin() { dontSpin_ = true; }
    
private:
    bool dontSpin_ = false;
    
    std::deque<T> datas_;
    
    SpinLock lock_;
};

