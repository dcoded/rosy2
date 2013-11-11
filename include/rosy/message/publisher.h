#ifndef __INCLUDE_MESSAGE_PUBLISHER_H__
#define __INCLUDE_MESSAGE_PUBLISHER_H__

#include "type.h"

#include <rosy/inbox.h>


namespace rosy {
namespace message {

template<int N>
class publisher : public dcoady::thread {
    private:
        dcoady::circular_fifo<subscriber*> channel_[N];
        inbox* inbox_;

    public:
        publisher(inbox& inbox)
        : inbox_(&inbox) {}

        bool subscribe(subscriber* sub, type type)
        {
            if(0 <= type && type < N)
            {
                return channel_[type].enqueue(sub);
            }
            return false;
        }

        void* run()
        {
            while(1)
            {
                if (inbox_->size() > 0)
                    publish(inbox_->pop ());
                usleep(1E5);
            }
            return NULL;
        }

        void publish(std::string message)
        {
            type t = static_cast<type> (message[0]);
            if(0 <= t && t < N)
            {
                size_t subscribers = channel_[t].size();
                while (subscribers-- > 0)
                {
                    subscriber* sub = channel_[t].dequeue();
                    sub->on_message(t, message);
                    channel_[t].enqueue(sub);
                }
            }
        }

};


}
}

#endif