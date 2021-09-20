#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <boost/asio.hpp>

#include "../../shadow/shadow_host_server_message.hpp"

class Client {
public:
    using AppInitialStatusCallback = std::function<void(std::vector<std::string>, shadow::ApplicationStatuses)>;
    using ConnectionCallback = std::function<void(bool)>;
    using ControlStatusCallback = std::function<void(bool)>;
    using SimulationStatusCallback = std::function<void(bool)>;
    using SimicsStatusCallback = std::function<void(bool)>;
    using VMListCallback = std::function<void(std::vector<std::string>, shadow::ApplicationStatuses, std::vector<std::vector<std::string>>,
                                              std::vector<std::vector<std::string>>)>;
    using VMRunningCallback = std::function<void(bool)>;
    Client(boost::asio::io_service &io_service, AppInitialStatusCallback app_initial_status_callback, ConnectionCallback connection_callback,
           ControlStatusCallback control_status_callback, SimulationStatusCallback simulation_status_callback,
           SimicsStatusCallback simics_status_callback, VMListCallback vm_list_callback, VMRunningCallback vm_running_callback);

    void start_simulation(std::uint32_t configuration);
    void stop_simulation();

    void start_simics();
    void pause_simics();

    void take_control();

private:
    void write_error_handler(const boost::system::error_code &error, std::size_t bytes_transferred);

    void reset();
    void reset_buffers();
    void start_async_connect();
    void start_connect();
    void start_read();

    shadow::ApplicationStatuses application_statuses;
    std::vector<std::string> application_names;

    shadow::ApplicationStatuses vm_running_list;
    std::vector<std::string> vm_list;

    boost::asio::io_service &io_service;
    boost::asio::ip::tcp::socket socket;
    boost::asio::ip::tcp::endpoint endpoint;

    boost::asio::streambuf header_buffer;
    boost::asio::streambuf data_buffer;

    bool connection_status = false;

    AppInitialStatusCallback app_initial_status_callback;
    ConnectionCallback connection_callback;
    ControlStatusCallback control_status_callback;
    SimulationStatusCallback simulation_status_callback;
    SimicsStatusCallback simics_status_callback;
    VMListCallback vm_list_callback;
    VMRunningCallback vm_running_callback;

    boost::asio::steady_timer timer;
    bool attempting_connection = false;
    bool timer_fired = false;
    bool connected = false;
};

#endif