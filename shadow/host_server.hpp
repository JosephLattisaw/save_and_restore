#ifndef HOST_SERVER_HPP
#define HOST_SERVER_HPP

#include <boost/asio.hpp>
#include <map>

#include "shadow_host_server_message.hpp"

class HostServer {
public:
    HostServer(boost::asio::io_service &io_service, std::uint16_t port, std::vector<std::string> application_names,
               shadow::ApplicationStatusesEnum application_statuses);
    ~HostServer();

private:
    class HostServerClient {
    public:
        using Disconnect_Callback = std::function<void(std::uint64_t)>;
        HostServerClient(std::uint64_t client_number, boost::asio::io_service &io_service, std::unique_ptr<boost::asio::ip::tcp::socket> sock,
                         Disconnect_Callback callback);

        void send_application_update(std::vector<std::string> application_names, shadow::ApplicationStatusesEnum application_statuses);

    private:
        void reset();
        void start_read();
        void reset_buffers();
        void write_error_handler(const boost::system::error_code &error, std::size_t bytes_transferred);

        bool in_control = false;
        std::unique_ptr<boost::asio::ip::tcp::socket> socket;
        boost::asio::streambuf message_buffer;
        boost::asio::io_service &io_service;
        Disconnect_Callback disconnect_callback;
        const std::uint64_t client_number;
        bool disconnected = false;
    };

    void start_async_accept();

    std::map<std::uint64_t, std::unique_ptr<HostServerClient>> socket_map;

    boost::asio::io_service &io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    const std::uint16_t port;
    std::unique_ptr<boost::asio::ip::tcp::socket> temp_socket;
    const std::vector<std::string> application_names;
    shadow::ApplicationStatusesEnum application_statuses;

    std::uint64_t client_number_count = 0;
};

#endif