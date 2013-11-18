#ifndef __INCLUDE_ROSY_MESSAGE_IMPL_NET_CONFIG_H__
#define __INCLUDE_ROSY_MESSAGE_IMPL_NET_CONFIG_H__

#include "message.h"

namespace rosy {
namespace message {

class net_config : public message<net_config, NET_CONF> {
public:
    MSGPACK_DEFINE(route, life, respond);
};


}
}
#endif