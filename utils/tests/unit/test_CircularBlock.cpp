#include <rapidcheck.h>

#include "utils/CircularBlock.h"

#include <thread>
#include <future>
#include <cassert>

#include <chrono>
using namespace std::chrono;

size_t cache_line_size() {
    FILE * p = 0;
    p = fopen("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size", "r");
    unsigned int i = 0;
    if (p) {
        auto ret = fscanf(p, "%d", &i);
        fclose(p);
    }
    return i;
}

template <typename T, size_t _BlockCapacity = 1024>
class rcCircularBlock : public CircularBlock<T,  _BlockCapacity>
{
public:
    size_t getSize() { return CircularBlock<T,  _BlockCapacity>::size_; }
};

int main()
{
    assert(cacheLinesSze == cache_line_size());
    
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Basic fill and then empty", [&]() 
    {
        rcCircularBlock<int> block;
        const auto nb = *rc::gen::inRange<size_t>(10, block.capacity()-1);
        
        for (auto i = 1UL; i <= nb; ++i)
        {
            block.fill(i);
        }
        RC_ASSERT(nb == block.getSize());
        for (auto i = 1UL; i <= nb; ++i)
        {
            RC_ASSERT(block.empty() == static_cast<int>(i));
        }
        RC_ASSERT(0UL == block.getSize());
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        for (auto i = 1UL; i <= nb; ++i)
        {
            block.fill(i);
        }
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        RC_ASSERT(nb == block.getSize());
        
        auto j = 0UL;
        start = high_resolution_clock::now();
        for (auto i = 1UL; i <= nb; ++i)
        {
            j = block.empty();
        }
        end = high_resolution_clock::now();
        time_span2 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        RC_ASSERT(0UL == block.getSize());
        RC_ASSERT(nb == j);
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Basic fill and then empty perfs [" << time_span1/nbTests 
            << '|' << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Dual threads fill and empty", [&]() 
    {
        std::promise<void> thr1_ready;
        std::promise<void> thr2_ready;
        std::promise<void> ready;
        std::shared_future<void> start(ready.get_future());
        
        rcCircularBlock<int> block;
        const auto nb = *rc::gen::inRange<size_t>(10, block.capacity()-1);
        
        auto threaded_emptyBlock = [&]() 
        {
            thr1_ready.set_value();
            start.wait();
            auto j = 0UL;
            high_resolution_clock::time_point start = high_resolution_clock::now();
            for (auto i = 1UL; i <= nb; ++i)
            {
                j = block.empty();
            }
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span2 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        };
        std::thread thr1(threaded_emptyBlock);

        auto threaded_fillBlock = [&]() 
        {
            thr2_ready.set_value();
            start.wait();
            high_resolution_clock::time_point start = high_resolution_clock::now();
            for (auto i = 1UL; i <= nb; ++i)
            {
                block.fill(int(i));
            }
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        };
        std::thread thr2(threaded_fillBlock);
        
//        block.dontSpin();
        thr1_ready.get_future().wait();
        thr2_ready.get_future().wait();
        ready.set_value();
        thr1.join();
        thr2.join();
        RC_ASSERT(0UL == block.getSize());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Dual threads fill and empty perfs [" << time_span1/nbTests 
            << '|' << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    time_span1 = 0ULL, time_span2 = 0ULL;
    nbTests = 0U;
    rc::check("Dual threads fill and empty with aligned to cache lines (false sharing)", [&]() 
    {
        std::promise<void> thr1_ready;
        std::promise<void> thr2_ready;
        std::promise<void> ready;
        std::shared_future<void> start(ready.get_future());
        
        typedef struct { char pad1[cacheLinesSze]; int data_ = 0; char pad2[cacheLinesSze]; } Data;
        rcCircularBlock<Data> block;
        const auto nb = *rc::gen::inRange<size_t>(10, block.capacity()-1);
        
        auto threaded_emptyBlock = [&]() 
        {
            thr1_ready.set_value();
            start.wait();
            auto j = Data();
            high_resolution_clock::time_point start = high_resolution_clock::now();
            for (auto i = 1UL; i <= nb; ++i)
            {
                j = block.empty();
            }
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span2 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        };
        std::thread thr1(threaded_emptyBlock);
        
        auto threaded_fillBlock = [&]() 
        {
            thr2_ready.set_value();
            start.wait();
            high_resolution_clock::time_point start = high_resolution_clock::now();
            for (auto i = 1UL; i <= nb; ++i)
            {
                Data d; d.data_ = i;
                block.fill(std::move(d));
            }
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span1 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        };
        std::thread thr2(threaded_fillBlock);
        
//        block.dontSpin();
        thr1_ready.get_future().wait();
        thr2_ready.get_future().wait();
        ready.set_value();
        thr1.join();
        thr2.join();
        RC_ASSERT(0UL == block.getSize());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Dual threads fill and empty (false sharing) perfs [" << time_span1/nbTests 
            << '|' << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    
    return 0;
}
