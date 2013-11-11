#ifndef __INCLUDE_ROSY_PACKET_TYPES_H__
#define __INCLUDE_ROSY_PACKET_TYPES_H__


namespace rosy {
namespace message {


enum type
{
    NET_SET,
    NET_DROP,

    DB_SET,
    DB_GET,

    PS_PUB,
    PS_SUB,
};

const static int type_count = 6;


}
}

#endif