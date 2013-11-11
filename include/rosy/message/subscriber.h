#ifndef __INCLUDE_MESSAGE_SUBSCRIBER_H__
#define __INCLUDE_MESSAGE_SUBSCRIBER_H__

#include "type.h"


namespace rosy {
namespace message {

class subscriber {
    public:
        virtual void on_message(type t, std::string) = 0;
};



}
}


#endif