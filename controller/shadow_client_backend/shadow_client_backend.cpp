#include <boost/asio.hpp>
#include <iostream>

#include "client.hpp"

// Global Variables
// These are needed for the actual backend
// Creating an actual object within frontend is overtly complicated and still being developed within
// the language itself. No one should be accessing these outside of the library itself
boost::asio::io_service io_service;

int main(int argc, char** argv) {
    boost::asio::io_service io_service;
    Client client(io_service,
                  [&](std::vector<std::string> names, shadow::ApplicationStatuses statuses) {});

    io_service.run();
    return 0;
}