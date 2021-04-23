#include "shadow.hpp"

#include <iostream>

Shadow::Shadow(std::vector<std::string> an, std::vector<std::string> sp, std::vector<bool> aho, std::vector<std::vector<bool>> cfgs,
               std::vector<std::vector<std::string>> args, std::string kp, boost::asio::io_service &io_service)
    : application_names(an), script_paths(sp), always_host_only(aho), configurations(cfgs), arguments(args), kill_path(kp), io_service(io_service) {
    // start_all_processes();
    application_statuses = shadow::ApplicationStatusesEnum(application_names.size(), shadow::shadow_app_status::app_status::NOT_RUNNING);
    host_server = std::make_unique<HostServer>(
        io_service, 20001, application_names, application_statuses,
        [&](bool power_sim, std::uint32_t configuration_number) {
            if (power_sim && !processing_kill) {
                if (configuration_number < configurations.size()) {
                    for (auto i = 0; i < script_paths.size() && i < application_names.size(); i++) {
                        children.push_back(std::make_shared<boost::process::child>(
                            script_paths[i], boost::process::std_out > boost::process::null, boost::process::std_err > boost::process::null,
                            boost::process::on_exit([&, i](int exit_code, const std::error_code &ec) {
                                std::cout << "process: " << application_names[i].c_str() << ", exited: " << exit_code << std::endl;
                                children[i].reset();
                            }),
                            io_service));
                    }
                    host_server->update_sim(true);
                    host_server->send_status_update();
                } else
                    std::cerr << "shadow: invalid configuration number received" << std::endl;
            } else if (!processing_kill) {
                // first we call kill script
                processing_kill = true;
                kill_child = std::make_shared<boost::process::child>(
                    kill_path, boost::process::std_out > boost::process::null, boost::process::std_err > boost::process::null,
                    boost::process::on_exit([&](int exit_code, const std::error_code &ec) {
                        std::cout << "kill process: " << kill_path << ", exited: " << exit_code << std::endl;
                        kill_child.reset();

                        // extra clean up just in case
                        for (auto i : children) {
                            if (i && i->valid() && i->running()) {
                                i->terminate();
                                i->wait();
                            }
                        }
                        children.clear();
                        std::cout << "cleaned up children, updating clients" << std::endl;
                        processing_kill = false;
                        host_server->update_sim(false);
                        host_server->send_status_update();
                    }),
                    io_service);
            }
        },
        [&](bool simics_sim) {
            if (simics_sim) {
                // TODO
            } else {
                // TODO
            }

            host_server->send_status_update();
        });
}

Shadow::~Shadow() {}