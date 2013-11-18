#ifndef __INCLUDE_ROSY_MESSAGE_IMPL_CONNECTION_H__
#define __INCLUDE_ROSY_MESSAGE_IMPL_CONNECTION_H__

#include <string>

namespace rosy {
namespace message {

struct connection {
    std::string from;
    std::string to;

    MSGPACK_DEFINE(from, to);
};


}
}

#endif