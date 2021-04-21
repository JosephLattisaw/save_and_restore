#include "shadow.hpp"

#include <iostream>

Shadow::Shadow(std::vector<std::string> app_names, std::vector<std::string> sp, std::vector<bool> aho, boost::asio::io_service &io_service)
    : application_names(app_names), script_paths(sp), always_host_only(aho), io_service(io_service) {
    // start_all_processes();
    application_statuses = shadow::ApplicationStatusesEnum(application_names.size(), shadow::shadow_app_status::app_status::NOT_RUNNING);
    host_server = std::make_unique<HostServer>(io_service, 20001, application_names, application_statuses);
}

Shadow::~Shadow() {}

void Shadow::start_all_processes() {
    for (auto i = 0; i < script_paths.size(); i++) {
        children.push_back(std::make_shared<boost::process::child>(
            "/home/efsi/projects/jwst/repos/jwst.git/shadow/scripts/launch_tcts_mtts_sim", boost::process::std_out > boost::process::null,
            boost::process::std_err > boost::process::null, boost::process::on_exit([&, i](int exit_code, const std::error_code &ec) {
                std::cout << "process exited: " << exit_code << std::endl;
                children[i].reset();
            }),
            io_service));
    }
}