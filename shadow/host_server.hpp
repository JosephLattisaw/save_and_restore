#ifndef HOST_SERVER_HPP
#define HOST_SERVER_HPP

#include <boost/asio.hpp>
#include <map>

#include "shadow_host_server_message.hpp"

class HostServer {
public:
    using PowerSimCallback = std::function<void(bool status, std::uint32_t configuration_number)>;
    using SimicsControlCallback = std::function<void(bool status)>;
    HostServer(boost::asio::io_service &io_service, std::uint16_t port, std::vector<std::string> application_names,
               shadow::ApplicationStatusesEnum application_statuses, PowerSimCallback power_sim_callback,
               SimicsControlCallback simics_control_callback);
    ~HostServer();

    void send_status_update();

    void update_simics(bool status) { simics_playing = status; }
    void update_sim(bool status) { sim_started = status; }

private:
    class HostServerClient {
    public:
        using Disconnect_Callback = std::function<void(std::uint64_t)>;
        using TakeControlCallback = std::function<void(std::uint64_t)>;
        using PowerSimCallback = std::function<void(std::uint64_t, bool status, std::uint32_t configuration)>;
        using SimicsControlCallback = std::function<void(std::uint64_t, bool status)>;
        HostServerClient(std::uint64_t client_number, boost::asio::io_service &io_service, std::unique_ptr<boost::asio::ip::tcp::socket> sock,
                         Disconnect_Callback disconnect_callback, PowerSimCallback power_sim_callback, TakeControlCallback take_control_callback,
                         SimicsControlCallback simics_control_callback);

        void send_application_update(std::vector<std::string> application_names, shadow::ApplicationStatusesEnum application_statuses);
        void send_status_update(std::uint64_t client_id, bool sim_started, bool simics_playing);

    private:
        void reset();
        void start_read();
        void reset_buffers();
        void write_error_handler(const boost::system::error_code &error, std::size_t bytes_transferred);

        bool in_control = false;
        std::unique_ptr<boost::asio::ip::tcp::socket> socket;

        boost::asio::io_service &io_service;

        boost::asio::streambuf header_buffer;
        boost::asio::streambuf data_buffer;

        const std::uint64_t client_number;
        bool disconnected = false;

        Disconnect_Callback disconnect_callback;
        PowerSimCallback power_sim_callback;
        TakeControlCallback take_control_callback;
        SimicsControlCallback simics_control_callback;
    };

    void start_async_accept();

    std::map<std::uint64_t, std::unique_ptr<HostServerClient>> socket_map;

    boost::asio::io_service &io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    const std::uint16_t port;
    std::unique_ptr<boost::asio::ip::tcp::socket> temp_socket;
    const std::vector<std::string> application_names;
    shadow::ApplicationStatusesEnum application_statuses;

    std::uint64_t client_number_count = 1;
    std::uint64_t client_in_control_id = 0;

    PowerSimCallback power_sim_callback;
    SimicsControlCallback simics_control_callback;

    bool sim_started = false;
    bool simics_playing = false;
};

#endif