#ifndef INCLUDE_THREAD
#define INCLUDE_THREAD

// $Id$
/**
 * @file thread.h
 * @author Denis Coady
 */
// $Log$

#include <pthread.h>
#include <unistd.h>

namespace dcoady {

/** @class thread

    @brief OOP abstract POSIX threading class similar to Java.

    A Java-like interface for building threadable classes in a constant
    manner.  The main loop of the thread is implemented by children as
    a run() capable of returning an arbitrary block of memory as a void*.

    Original design of class can be found at:
    http://vichargrave.com/java-style-thread-class-in-c/


    @author Vic Hargrave
    @date December 2012
*/
class thread {
public:

    /** Default constructor.*/
    thread();

    /** Abstract deconstructor */
    virtual ~thread();
 
    /** Create and start the posix thread */
    int start();

    /** Join the thread and wait for completion */
    int join();

    /** Detatch the thread as a daemon */
    int detach();

    /** Returns an auto-generated thread id */
    pthread_t self() const;

    /** Entry/Exit point of thread, implemented by child class
        
        @param arg - The method pointer of run() implementation
        @return a pointer to an arbitrary block of memory
    */
    static void* run(void* arg);
 
    /** Abstract interface for child run() */
    virtual void* run() = 0;
 
private:

    pthread_t  id_; /**<POSIX thread object */
    int        running_;/**<Running state of thread */
    int        detached_;/**<Detatched state of thread */
};

};

#endif