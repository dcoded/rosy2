#ifndef __INCLUDE_ROSY_MESSAGE_H__
#define __INCLUDE_ROSY_MESSAGE_H__

#include <msgpack.h>
#include <string>

#include "type.h"

namespace rosy {
namespace message {

template<enum type T, class A>
class message {
public:
    virtual std::string  pack()
    {
        msgpack::sbuffer sbuf;
        msgpack::pack (sbuf, *((A*) this));

        return static_cast<char> (T) + std::string(sbuf.data (), sbuf.size ());
    }

    virtual void unpack(std::string& buf)
    {
        msgpack::unpacked msg;
        msgpack::object   obj;

        msgpack::unpack(&msg, buf.data() + 1, buf.size() - 1);
 
        obj = msg.get();
        obj.convert( (A*)this );
    }

public:
};



class net_set : public message<NET_SET, net_set> {
private:
    std::string origin_;
    std::string to_;
    std::map<std::string, std::string> table_;
public:
    net_set(){}
    net_set(const char* origin, const char* to)
    : origin_(origin)
    , to_(to) {}

    net_set(std::string origin, std::string to)
    : origin_(origin)
    , to_(to) {}

    const std::string origin() const { return origin_; }
    const std::string to()     const { return to_; }
    std::map<std::string, std::string>& table() { return table_; }

    MSGPACK_DEFINE(origin_, to_, table_);
};


class net_drop : public message<NET_DROP, net_drop> {
private:
    std::string origin_;
    std::string bad_;
    std::string next_;
public:
    net_drop(){}
    net_drop(const char* origin, const char* bad, const char* next)
    : origin_(origin)
    , bad_(bad)
    , next_(next) {}

    net_drop(std::string origin, std::string bad, std::string next)
    : origin_(origin)
    , bad_(bad)
    , next_(next) {}

    const std::string bad() const { return bad_; }
    const std::string origin() const { return origin_; }
    const std::string next() const { return next_; }


    MSGPACK_DEFINE(bad_, origin_, next_);
};

// class net_ack : public message<NET_ACK, net_ack> {
// private:
//     std::string joinee_;
//     std::string from_;
//     std::string to_;

// public:
//     net_ack(){}
//     net_ack(const char* joinee, const char* from, const char* to)
//     : joinee_(joinee)
//     , from_(from)
//     , to_(to) {}

//     net_ack(std::string joinee, std::string from, std::string to)
//     : joinee_(joinee)
//     , from_(from)
//     , to_(to) {}

//     const std::string joinee() const { return joinee_; }
//     const std::string from()   const { return from_; }
//     const std::string to()     const { return to_; }

//     MSGPACK_DEFINE(joinee_, from_, to_);
// };


// class net_add : public message<NET_ADD, net_add> {
// private:
//     std::string self_endpoint;
//     std::string next_endpoint;

// public:
//     net_add(){}
//     net_add(const char* self, const char* next)
//     : self_endpoint(self)
//     , next_endpoint(next) {}

//     net_add(std::string self, std::string next)
//     : self_endpoint(self)
//     , next_endpoint(next) {}

//     const std::string self() const { return self_endpoint; }
//     const std::string next() const { return next_endpoint; }

//     MSGPACK_DEFINE(self_endpoint, next_endpoint);
// };


//class ack : public message<NET_ACK>;




}
}
#endif