#include <dcoady/thread.h>

namespace dcoady {

thread::thread()
: id_(0)
, running_(0)
, detached_(0) {}


thread::~thread()
{
    if (running_ == 1) {
        pthread_cancel(id_);
    }
    if (running_ == 1 && detached_ == 0) {
        pthread_detach(id_);
    }
}

void* thread::run (void* arg)
{
    thread* runnable = static_cast<thread*>(arg);
    return  runnable->run();
}
 
int thread::start()
{
    int result = pthread_create(&id_, NULL, thread::run, this);
    if (result == 0)
    {
        running_ = 1;
    }
    return result;
}

int thread::join()
{
    int result = -1;
    if (running_ == 1)
    {
        result = pthread_join(id_, NULL);
        
        if (result == 0)
            detached_ = 1;
        
    }
    return result;
}

int thread::detach()
{
    int result = -1;
    if (running_ == 1 && detached_ == 0)
    {
        result = pthread_detach(id_);

        if (result == 0)
            detached_ = 1;
    }
    return result;
}

pthread_t thread::self() const
{
    return id_;
}

void thread::notify()
{
    pthread_cond_signal(&cond_);
}

void thread::wait()
{
    pthread_cond_wait(&cond_, &mutex_);   
}


};