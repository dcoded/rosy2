#ifndef INCLUDE_CIRCULAR_QUEUE
#define INCLUDE_CIRCULAR_QUEUE

// $Id$
/**
 * @file circular_fifo.h
 * @author Denis Coady
 */
// $Log$

#include <pthread.h>
#include <stdexcept>

#ifndef NULL
#define NULL 0
#endif

namespace dcoady {

/** @class circular_fifo

    @brief A thread-safe circular fifo queue

    @author Denis Coady
    @date May 2013
*/
template<typename T, int MAX_SIZE = 128>
class circular_fifo {
public:

    /** Default constructor */
    circular_fifo()
    : begin_(array_)
    , end_(array_)
    , size_(0)
    {
        pthread_mutex_init(&enqueue_lock_, NULL);
        pthread_mutex_init(&dequeue_lock_, NULL);
    }

    /* Default deconstructor */
    ~circular_fifo()
    {
        pthread_mutex_destroy(&enqueue_lock_);
        pthread_mutex_destroy(&dequeue_lock_);
    }

    /** Add a new element to the end of the queue
        @param element - Element to be added
        @return -1 if queue is full, else returns position of element
    */
    int enqueue(T element)
    {
        int pos;
        pthread_mutex_lock(&enqueue_lock_);

        if(size_ >= MAX_SIZE)
        {
            pos = -1;
        }
        else
        {
            *end_ = element;
             end_ = increment_ptr(end_);

            pos = size_;
            size_++;
        }

        pthread_mutex_unlock(&enqueue_lock_);
        return pos;
    }

    /** Removes and returns element at front of queue
        @return the element at front of queue
    */
    T dequeue()
    {
        pthread_mutex_lock(&dequeue_lock_);
        if(size_ < 0)
            throw std::runtime_error("dequeue() invalid on an empty queue");

        T* val = begin_;
        begin_ = increment_ptr(begin_);
        size_--;
        pthread_mutex_unlock(&dequeue_lock_);


        return *val;
    }

    /** Returns element at front of queue
        @return the element at front of queue
    */
    T front ()
    {
        pthread_mutex_lock(&dequeue_lock_);
        if(size_ < 0)
            throw std::runtime_error("front() invalid on an empty queue");

        T val = *begin_;
        pthread_mutex_unlock(&dequeue_lock_);

        return val;
    }

    /** Returns if queue is full
        @return if queue is full
    */
    bool full() const
    {
        return (size_ == MAX_SIZE);
    }

    /** Returns if queue is empty
        @return if queue is empty
    */
    bool empty() const
    {
        return (size_ == 0);
    }

    /** Returns size of queue
        @return size of queue
    */
    size_t size() const
    {
        return size_;
    }

private:

    /** fixed size array of elements */
    T array_[MAX_SIZE+1];

    /** head pointer of circular buffer */
    T* begin_;

    /** tail pointer of circular buffer */
    T* end_;

    /** the number of elements in the queue */
    size_t size_;

    /** POSIX mutex locks */
    pthread_mutex_t dequeue_lock_;
    pthread_mutex_t enqueue_lock_;

    /** Increments pointer to next element
        @param ptr - Head or tail pointer
        @return the next element in the buffer
    */
    T* increment_ptr(T* ptr)
    {
        T*  tmp = ptr;
        if (tmp < array_ + MAX_SIZE)
            tmp++;
        else
            tmp = array_;

        return tmp;
    }
};

};

#endif