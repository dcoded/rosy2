// #ifndef __INCLUDE_ROSY_TOPOLOGY_H__
// #define __INCLUDE_ROSY_TOPOLOGY_H__

// #include <iostream>
// #include <map>
// #include <string>

// #include <msgpack.hpp>

// #include <dcoady/thread.h>

// #include <rosy/message/subscriber.h>
// #include <rosy/message/message.h>
// #include <rosy/inbox.h>
// #include <rosy/outbox.h>


// namespace rosy {

// class topology : public dcoady::thread,
//                  public message::subscriber {

//     private:
//         std::map<std::string, std::string> endpoints_;
//         inbox* inbox_;
//         outbox* outbox_;

//     public:
//         topology (inbox* inbox, outbox* outbox);

//         void* run();

//         void recover(std::string new_endpoint);

//         void on_message(message::type type, std::string msg);

//         void set_endpoint (message::net_set& message);

//         void drop_endpoint(message::net_drop& message);


//         void print_table();

// };

// }




// #endif