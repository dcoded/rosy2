#ifndef __INCLUDE_ROSY_MESSAGE_H__
#define __INCLUDE_ROSY_MESSAGE_H__

#include <sstream>
#include <string>
#include <cstdlib>

#include "uuid.h"

namespace rosy {

class message {
private:
    const char        type_;
    const std::string from_;
    const std::string recp_;
    const std::string data_;

public:
    message()
    : type_(0xFF)
    , from_(uuid::zero ())
    , recp_(uuid::zero ())
    , data_("") {}

    message(char type, std::string uuid_from,
                            std::string uuid_recp,
                            std::string data)
    : type_(type)
    , from_(uuid_from)
    , recp_(uuid_recp)
    , data_(data.data ()) {}

    static std::string pack (char type, std::string uuid_from,
                                        std::string uuid_recp,
                                        std::string data)
    {
        message msg(type, uuid_from, uuid_recp, data);
        return msg.pack ();
    }

    static message unpack (std::string packed)
    {
        return message
        (
            packed[0],
            packed.substr (1, 36),
            packed.substr (37, 36),
            packed.substr (73)
        );
    }

    std::string pack() const
    {
        std::stringstream ss;

        ss << type_;
        ss << from_;
        ss << recp_;
        ss << data_;

        return ss.str ();
    }

    const char         type () { return type_; }
    const std::string  from () { return from_; }
    const std::string  recp () { return recp_; }
    const size_t       size () { return data_.size (); }
    const std::string  data () { return data_; }
};

}


#endif