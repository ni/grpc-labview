//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
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
