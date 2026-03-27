//---------------------------------------------------------------------
// lv_semaphore.h — lightweight counting semaphore for grpc-labview.
//
// Renamed from "semaphore.h" to avoid shadowing the POSIX <semaphore.h>
// system header on Linux (GCC's <condition_variable> transitively
// includes <bits/semaphore_base.h> which needs <semaphore.h>).
//---------------------------------------------------------------------
#pragma once

#include <condition_variable>
#include <mutex>

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class Semaphore
    {
    public:
        Semaphore (int count_ = 1)
            : count(count_)
        {        
        }

        inline void notify()
        {
            std::unique_lock<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }

        inline void wait()
        {
            std::unique_lock<std::mutex> lock(mtx);

            while(count == 0)
            {
                cv.wait(lock);
            }
            count--;
        }

    private:
        std::mutex mtx;
        std::condition_variable cv;
        int count;
    };
}
