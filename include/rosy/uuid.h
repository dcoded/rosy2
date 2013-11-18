#ifndef __INCLUDE_ROSY_UUID_H__
#define __INCLUDE_ROSY_UUID_H__

#include <fstream>
#include <iostream>

namespace rosy {

class uuid {
    static std::ifstream uuid_;

public:
    static bool generate (char* buf);
    static std::string generate ();
    static std::string zero ();

};

}
#endif