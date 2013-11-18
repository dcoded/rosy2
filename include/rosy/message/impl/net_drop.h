#ifndef __INCLUDE_ROSY_MESSAGE_IMPL_NET_DROP_H__
#define __INCLUDE_ROSY_MESSAGE_IMPL_NET_DROP_H__


#include "message.h"

namespace rosy {
namespace message {

class net_drop : public message<net_drop, NET_DROP> {
public:
    std::string target;
    MSGPACK_DEFINE(route, respond, target);
};


}
}
#endif