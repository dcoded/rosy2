#include <rosy/uuid.h>

namespace rosy {

std::ifstream uuid::uuid_;

bool uuid::generate (char* buf)
{
    *buf = 0x0;

    if (!uuid_.is_open())
         uuid_.open("/proc/sys/kernel/random/uuid");

    if (uuid_.good())
        uuid_.getline (buf, 37);

    return (*buf != 0x0);
}

std::string uuid::generate ()
{
    char buf[36];
    generate (buf);

    return std::string(buf, 36);
}

std::string uuid::zero ()
{
    return "00000000-0000-0000-0000-000000000000";
}

}