#ifndef SHADOW_HPP
#define SHADOW_HPP

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include "host_server.hpp"
#include "shadow_host_server_message.hpp"

class Shadow {
public:
    Shadow(std::vector<std::string> application_names, std::vector<std::string> script_paths, std::vector<bool> always_host_only,
           boost::asio::io_service &io_service);
    ~Shadow();

    void start_all_processes();

private:
    // configuration
    std::vector<bool> always_host_only;
    std::vector<std::string> script_paths;
    std::vector<std::string> application_names;
    shadow::ApplicationStatusesEnum application_statuses;

    std::vector<std::shared_ptr<boost::process::child>> children;  // child processes

    std::unique_ptr<HostServer> host_server;

    boost::asio::io_service &io_service;
};

#endif