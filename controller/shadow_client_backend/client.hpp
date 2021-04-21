#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio.hpp>

#include "../../shadow/shadow_host_server_message.hpp"

class Client {
public:
    using AppInitialStatusCallback =
        std::function<void(std::vector<std::string>, shadow::ApplicationStatuses)>;
    Client(boost::asio::io_service &io_service,
           AppInitialStatusCallback app_initial_status_callback);

private:
    void reset();
    void reset_buffers();
    void start_async_connect();
    void start_read();

    void update_connection_status(bool status);

    shadow::ApplicationStatuses application_statuses;
    std::vector<std::string> application_names;

    boost::asio::io_service &io_service;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::endpoint endpoint;

    boost::asio::streambuf header_buffer;
    boost::asio::streambuf data_buffer;

    AppInitialStatusCallback app_initial_status_callback;
};

#endif