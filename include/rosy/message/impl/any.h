#ifndef __INCLUDE_ROSY_MESSAGE_IMPL_ANY_H__
#define __INCLUDE_ROSY_MESSAGE_IMPL_ANY_H__

#include "message.h"

namespace rosy {
namespace message {

class any : public message<any, ANY> {
public:
    MSGPACK_DEFINE(route);

    bool is_recipient (std::string addr) const
    {
        return (route.to == addr || route.to == "");
    }

    bool is_owner (std::string addr) const
    {
        return (route.from == addr);
    }
};


}
}

#endif