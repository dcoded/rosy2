#ifndef __INCLUDE_ROSY_MESSAGE_IMPL_RAW_H__
#define __INCLUDE_ROSY_MESSAGE_IMPL_RAW_H__

#include <map>

#include "message.h"

namespace rosy {
namespace message {

class raw : public message<raw, RAW> {
public:
    std::string data;
    MSGPACK_DEFINE(route, respond, data);
};


}
}
#endif