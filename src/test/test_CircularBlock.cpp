#include <rapidcheck.h>

#include "CircularBlock.h"

#include <thread>
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
    const int cacheLineSize = cache_line_size();
    std::cout << "Cache line is " << cacheLineSize << std::endl;
    assert(cacheLineSize == 64);
    
    auto time_span1 = 0ULL, time_span2 = 0ULL;
    auto nbTests = 0U;
    rc::check("Basic fill and then empty", [&]() 
    {
        rcCircularBlock<int> block;
        const auto nb = *rc::gen::inRange<size_t>(10, block.capacity()-1);
        
        high_resolution_clock::time_point start = high_resolution_clock::now();
        for (auto i = 1UL; i <= nb; ++i)
        {
            block.fill(int(i));
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
        CircularBlock<int, 2> waitThread;
        rcCircularBlock<int> block;
        const auto nb = *rc::gen::inRange<size_t>(10, block.capacity()-1);
        
        auto threaded_emptyBlock = [&]() 
        {
            waitThread.fill(0); // tell thread ready
            auto j = 0UL;
            high_resolution_clock::time_point start = high_resolution_clock::now();
            for (auto i = 1UL; i <= nb; ++i)
            {
                j = block.empty();
            }
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span2 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        };
        std::thread thr(threaded_emptyBlock);

        waitThread.empty(); // wait thread
        high_resolution_clock::time_point start = high_resolution_clock::now();
        for (auto i = 1UL; i <= nb; ++i)
        {
            block.fill(int(i));
        }
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        
//        block.dontSpin();
        thr.join();
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
        CircularBlock<int, 2> waitThread;
//        typedef struct alignas(64) { int data_ = 0; } Data;
        typedef struct { int pad1[8]; int data_ = 0; int pad2[7]; } Data;
        rcCircularBlock<Data> block;
        const auto nb = *rc::gen::inRange<size_t>(10, block.capacity()-1);
        
        auto threaded_emptyBlock = [&]() 
        {
            waitThread.fill(0); // tell thread ready
            auto j = Data();
            high_resolution_clock::time_point start = high_resolution_clock::now();
            for (auto i = 1UL; i <= nb; ++i)
            {
                j = block.empty();
            }
            high_resolution_clock::time_point end = high_resolution_clock::now();
            time_span2 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        };
        std::thread thr(threaded_emptyBlock);

        waitThread.empty(); // wait thread
        high_resolution_clock::time_point start = high_resolution_clock::now();
        for (auto i = 1UL; i <= nb; ++i)
        {
            Data d; d.data_ = i;
            block.fill(std::move(d));
        }
        high_resolution_clock::time_point end = high_resolution_clock::now();
        time_span1 += (duration_cast<nanoseconds>(end - start).count()) / nb;
        
//        block.dontSpin();
        thr.join();
        RC_ASSERT(0UL == block.getSize());
        
        ++nbTests;
    });
    if (nbTests)
    {
        std::cout << "Dual threads fill and empty perfs [" << time_span1/nbTests 
            << '|' << time_span2/nbTests << "] (in ns)" << std::endl;
    }
    
    
    return 0;
}
