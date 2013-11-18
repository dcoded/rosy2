#ifndef __INCLUDE_ROSY_MESSAGE_IMPL_MESSAGE_H__
#define __INCLUDE_ROSY_MESSAGE_IMPL_MESSAGE_H__

#include <msgpack.hpp> // make sure .hpp and not .h!
#include <string>
#include <rosy/uuid.h>

#include "type.h"
#include "connection.h"
#include <iostream>

namespace rosy {
namespace message {



template<class A, enum type T>
class message {
public:
    int life;
    connection  route; // routing info, who is this packet for?
    bool        respond;

    message() {}
    virtual std::string pack()
    {
        msgpack::sbuffer sbuf;
        msgpack::pack (sbuf, *((A*) this));

        std::string packed = static_cast<char> (T) + std::string(sbuf.data (), sbuf.size ());
        return packed;
    }

    virtual void unpack(std::string& buf)
    {
        msgpack::unpacked msg;
        msgpack::object   obj;

        msgpack::unpack(&msg, buf.data() + 1, buf.size() - 1);
 
        obj = msg.get();
        obj.convert( (A*)this );
    }
};













}
}
#endif