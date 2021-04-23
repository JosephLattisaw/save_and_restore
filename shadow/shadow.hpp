#ifndef SHADOW_HPP
#define SHADOW_HPP

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include "host_server.hpp"
#include "shadow_host_server_message.hpp"

class Shadow {
public:
    Shadow(std::vector<std::string> application_names, std::vector<std::string> script_paths, std::vector<bool> always_host_only,
           std::vector<std::vector<bool>> configurations, std::vector<std::vector<std::string>> arguments, std::string kill_path,
           boost::asio::io_service &io_service);
    ~Shadow();

private:
    // configuration
    std::vector<bool> always_host_only;
    std::vector<std::string> script_paths;
    std::vector<std::string> application_names;
    shadow::ApplicationStatusesEnum application_statuses;
    std::vector<std::vector<bool>> configurations;
    std::vector<std::vector<std::string>> arguments;
    std::string kill_path;

    std::vector<std::shared_ptr<boost::process::child>> children;  // child processes
    std::shared_ptr<boost::process::child> kill_child;

    std::unique_ptr<HostServer> host_server;

    boost::asio::io_service &io_service;
    bool processing_kill = false;
};

#endif