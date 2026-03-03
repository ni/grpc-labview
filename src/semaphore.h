//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
// On GCC/Clang (Linux/Mac), our file name "semaphore.h" collides with the
// POSIX <semaphore.h> system header. The GCC C++11 <condition_variable>
// header transitively includes bits/semaphore_base.h, which does
// #include <semaphore.h> expecting POSIX sem_t. Because ./src is on the
// -I path, it finds this file instead. #pragma once then blocks re-entry,
// leaving sem_t undefined and std::condition_variable unresolvable.
//
// #include_next forwards to the *next* semaphore.h in the include search
// path (i.e. the real POSIX one), pre-defining sem_t before
// <condition_variable> triggers the circular chain.
//---------------------------------------------------------------------
#ifdef __GNUC__
#include_next <semaphore.h>
#endif

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
