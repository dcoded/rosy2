#ifndef __INCLUDE_ROSY_MESSAGE_IMPL_JOIN_H__
#define __INCLUDE_ROSY_MESSAGE_IMPL_JOIN_H__

#include <map>

#include "message.h"

namespace rosy {
namespace message {

class join : public message<join, JOIN> {
public:
    connection next;

    MSGPACK_DEFINE(next);
};


}
}
#endif