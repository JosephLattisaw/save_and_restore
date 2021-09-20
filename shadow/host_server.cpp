#include "host_server.hpp"

#include <iostream>

#include "shadow_host_server_message.hpp"

HostServer::HostServer(boost::asio::io_service& io_service, std::uint16_t port, std::vector<std::string> app_names,
                       shadow::ApplicationStatusesEnum app_statuses, PowerSimCallback ps_cb, SimicsControlCallback sc_cb)
    : io_service(io_service),
      port(port),
      acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      application_names(app_names),
      application_statuses(app_statuses),
      power_sim_callback{ps_cb},
      simics_control_callback{sc_cb} {
    fcntl(acceptor.native_handle(), F_SETFD, FD_CLOEXEC);  // this is needed to prevent any child process from taking control on crash
    start_async_accept();
}

HostServer::~HostServer() {}

void HostServer::start_async_accept() {
    assert(!temp_socket);
    temp_socket = std::make_unique<boost::asio::ip::tcp::socket>(io_service);
    acceptor.async_accept(*temp_socket, [&](const boost::system::error_code& error) {
        if (!error) {
            fcntl(temp_socket->native_handle(), F_SETFD, FD_CLOEXEC);
            std::cout << "HostServer: accepted connection on port " << port << std::endl;
            std::cout << "HostServer: assigned client id of: " << client_number_count << std::endl;
            auto c = std::make_unique<HostServerClient>(
                client_number_count, io_service, std::move(temp_socket),
                [&](std::uint64_t client_id) {
                    auto it = socket_map.find(client_id);
                    if (it != socket_map.end()) {
                        socket_map.erase(client_id);
                        if (client_in_control_id == client_id) {
                            client_in_control_id = -1;
                            send_status_update();
                        }
                        std::cout << "HostServer: client " << client_id << " disconnected" << std::endl;
                    } else
                        std::cerr << "HostServer: could not find socket to erase" << std::endl;
                },
                [&](std::uint64_t client_id, bool power_sim, std::uint32_t configuration_number) {
                    if (client_id == client_in_control_id) {
                        std::cout << "HostServer: received power sim: " << power_sim << ", config: " << configuration_number
                                  << ", from client in control" << std::endl;
                        power_sim_callback(power_sim, configuration_number);
                    } else
                        std::cerr << "HostServer: error received power sim: " << power_sim << "from client not in control, ignoring" << std::endl;
                },
                [&](std::uint64_t client_id) {
                    client_in_control_id = client_id;
                    std::cout << "HostServer: client " << client_in_control_id << " is now in control" << std::endl;
                    send_status_update();
                },
                [&](std::uint64_t client_id, bool simics_sim) {
                    if (client_id == client_in_control_id) {
                        std::cout << "HostServer: received simics control sim: " << simics_sim << "from client in control" << std::endl;
                        simics_control_callback(simics_sim);
                    } else
                        std::cerr << "HostServer: error received simics control sim: " << simics_sim << " from client not in control, ignoring"
                                  << std::endl;
                });

            socket_map.insert(std::make_pair(client_number_count, std::move(c)));

            // sanity check
            auto it = socket_map.find(client_number_count);
            assert(it != socket_map.end());

            it->second->send_application_update(application_names, application_statuses);
            it->second->send_vm_status_update(vm_list, vm_running_list, vm_snaps_list);

            client_number_count++;
        } else {
            std::cerr << "HostServer: attempted to accept connection on port " << port << " but an error occurred: " << error.message() << std::endl;
            temp_socket.reset();
            return;
        }

        // starting to accept connections again now that our connection has been processed
        start_async_accept();
    });
}

HostServer::HostServerClient::HostServerClient(std::uint64_t client_number, boost::asio::io_service& io_service,
                                               std::unique_ptr<boost::asio::ip::tcp::socket> sock, Disconnect_Callback d_cb, PowerSimCallback ps_cb,
                                               TakeControlCallback tc_cb, SimicsControlCallback sc_cb)
    : client_number(client_number),
      io_service(io_service),
      socket(std::move(sock)),
      disconnect_callback{d_cb},
      power_sim_callback{ps_cb},
      take_control_callback{tc_cb},
      simics_control_callback{sc_cb} {
    start_read();
}

void HostServer::HostServerClient::start_read() {
    assert(socket);
    boost::asio::async_read(
        *socket, header_buffer, boost::asio::transfer_exactly(sizeof(shadow::shadow_host_message)),
        [&](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (!error) {
                if (header_buffer.size() == sizeof(shadow::shadow_host_message)) {
                    const shadow::shadow_host_message* shm = boost::asio::buffer_cast<const shadow::shadow_host_message*>(header_buffer.data());
                    auto msg_size = shm->size;
                    auto msg = shm->message;
                    if (msg_size == 0) {
                        switch (msg) {
                            case shadow::shadow_host_message::host_message::TAKE_CONTROL:
                                take_control_callback(client_number);
                                break;
                            case shadow::shadow_host_message::host_message::START_SIM:
                                // power_sim_callback(client_number, true);
                                std::cerr << "host server: received start sim command without following message, resetting socket" << std::endl;
                                reset();
                                break;
                            case shadow::shadow_host_message::host_message::STOP_SIM:
                                power_sim_callback(client_number, false, 0);
                                break;
                            case shadow::shadow_host_message::host_message::PLAY_SIMICS:
                                simics_control_callback(client_number, true);
                                break;
                            case shadow::shadow_host_message::host_message::PAUSE_SIMICS:
                                simics_control_callback(client_number, false);
                                break;
                        }

                        reset_buffers();
                        start_read();
                    } else {
                        boost::asio::async_read(
                            *socket, data_buffer, boost::asio::transfer_exactly(msg_size),
                            [&, msg, msg_size](const boost::system::error_code& error, std::size_t bytes_transferred) {
                                if (!error) {
                                    if (data_buffer.size() == msg_size) {
                                        switch (msg) {
                                            case shadow::shadow_host_message::host_message::START_SIM:
                                                if (data_buffer.size() == sizeof(shadow::shadow_config_message)) {
                                                    const shadow::shadow_config_message* scm =
                                                        boost::asio::buffer_cast<const shadow::shadow_config_message*>(data_buffer.data());
                                                    power_sim_callback(client_number, true, scm->configuration_number);
                                                } else {
                                                    std::cerr
                                                        << "host server client: received invalid config message size following start sim command"
                                                        << std::endl;
                                                    reset();
                                                }

                                                break;
                                        }
                                        reset_buffers();
                                        start_read();
                                    } else {
                                        std::cerr << "client: received invalid data size of: " << bytes_transferred << ", expected : " << msg_size
                                                  << std::endl;
                                        reset();
                                    }
                                } else {
                                    std::cerr << "client: encountered error when reading data: " << error.message() << std::endl;
                                    reset();
                                }
                            });
                    }
                } else {
                    std::cerr << "host server client " << client_number << ": received invalid message size of: " << bytes_transferred
                              << ", expected: " << sizeof(shadow::shadow_host_message) << std::endl;
                    reset();
                }
            } else {
                std::cerr << "host server client " << client_number << ": encountered error when reading header : " << error.message() << std::endl;
                reset();
            }
        });
}

void HostServer::HostServerClient::reset_buffers() {
    header_buffer.consume(header_buffer.size());
    data_buffer.consume(data_buffer.size());
}

void HostServer::HostServerClient::reset() {
    if (socket) {
        socket->close();
        socket.release();
    }
    reset_buffers();
    if (!disconnected) disconnect_callback(client_number);
    disconnected = true;
}

void HostServer::HostServerClient::send_application_update(std::vector<std::string> application_names,
                                                           shadow::ApplicationStatusesEnum application_statuses) {
    assert(application_names.size() == application_statuses.size());
    assert(socket);
    if (socket && socket->is_open()) {
        // creating main header
        shadow::shadow_host_message shm;
        shm.message = shadow::shadow_host_message::host_message::APP_INITIAL_STATUS;

        // total applications
        shadow::shadow_total_apps sta;
        sta.total_applications = application_statuses.size();
        shm.size += sizeof(shadow::shadow_total_apps);

        std::vector<size_t> as_sizes;
        std::vector<std::vector<std::uint8_t>> names;

        for (auto i = 0; i < application_names.size(); i++) {
            std::vector<std::uint8_t> app_name(application_names[i].begin(), application_names[i].end());

            as_sizes.push_back(app_name.size() * sizeof(decltype(app_name)::value_type));
            names.push_back(app_name);

            shm.size += sizeof(shadow::shadow_app_status);
            shm.size += as_sizes.back();
        }

        // writing header
        boost::asio::async_write(*socket, boost::asio::buffer(reinterpret_cast<char*>(&shm), sizeof(shm)),
                                 std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

        // writing total applications
        boost::asio::async_write(*socket, boost::asio::buffer(reinterpret_cast<char*>(&sta), sizeof(sta)),
                                 std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

        for (auto i = 0; i < application_names.size(); i++) {
            shadow::shadow_app_status sat;
            sat.status = application_statuses[i];
            sat.size = as_sizes[i];

            boost::asio::async_write(
                *socket, boost::asio::buffer(reinterpret_cast<char*>(&sat), sizeof(sat)),
                std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

            boost::asio::async_write(
                *socket, boost::asio::buffer(reinterpret_cast<char*>(names[i].data()), as_sizes[i]),
                std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));
        }
    } else {
        std::cerr << "host server client: " << client_number
                  << ", socket wasn't open while attempting to "
                     "write, closing socket"
                  << std::endl;
        reset();
    }
}

void HostServer::HostServerClient::write_error_handler(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (error) {
        std::cerr << "host server client: " << client_number << " error writing data: " << error.message() << std::endl;
        reset();
    }
}

void HostServer::send_status_update() {
    for (auto const& [key, val] : socket_map) {
        val->send_status_update(client_in_control_id, sim_started, simics_playing);
    }
}

void HostServer::HostServerClient::send_status_update(std::uint64_t client_id, bool sim_started, bool simics_playing) {
    shadow::shadow_host_message shm;
    shm.size = sizeof(shadow::shadow_main_status);
    shm.message = shadow::shadow_host_message::host_message::MAIN_STATUS;

    shadow::shadow_main_status sms;
    sms.in_control = (client_id == client_number);
    sms.simulation_started = sim_started;
    sms.simics_playing = simics_playing;

    boost::asio::async_write(*socket, boost::asio::buffer(reinterpret_cast<char*>(&shm), sizeof(shm)),
                             std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

    boost::asio::async_write(*socket, boost::asio::buffer(reinterpret_cast<char*>(&sms), sizeof(sms)),
                             std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));
}

void HostServer::send_vm_status_update() {
    for (auto const& [key, val] : socket_map) {
        val->send_vm_status_update(vm_list, vm_running_list, vm_snaps_list);
    }
}

void HostServer::HostServerClient::send_vm_status_update(std::vector<std::string> vm_list, std::vector<std::uint8_t> vm_running_list,
                                                         std::vector<std::vector<std::vector<std::string>>> vm_snaps_list) {
    if (vm_list.size() == vm_running_list.size() && vm_snaps_list.size() == vm_list.size()) {
        if (socket && socket->is_open()) {
            // creating main header
            shadow::shadow_host_message shm;
            shm.message = shadow::shadow_host_message::host_message::VM_LIST;

            // total applications
            shadow::shadow_total_apps sta;
            sta.total_applications = vm_list.size();
            shm.size += sizeof(shadow::shadow_total_apps);

            std::vector<size_t> as_sizes;
            std::vector<std::vector<std::uint8_t>> names;

            for (auto i = 0; i < vm_list.size(); i++) {
                std::vector<std::uint8_t> app_name(vm_list[i].begin(), vm_list[i].end());

                as_sizes.push_back(app_name.size() * sizeof(decltype(app_name)::value_type));
                names.push_back(app_name);

                shm.size += sizeof(shadow::shadow_app_status);
                shm.size += as_sizes.back();
            }

            // snap sizes
            shadow::shadow_total_vm_snapshots stvms;
            shm.size += sizeof(shadow::shadow_total_vm_snapshots);
            std::vector<std::uint32_t> snap_sizes;

            for (auto i = 0; i < vm_snaps_list.size(); i++) {
                snap_sizes.push_back(vm_snaps_list[i].size());
            }
            stvms.size = snap_sizes.size();
            shm.size += snap_sizes.size() * sizeof(std::uint32_t);

            // snap names
            std::vector<std::vector<size_t>> as_sizes_3;
            std::vector<std::vector<std::vector<std::uint8_t>>> names_3;

            std::vector<std::vector<size_t>> as_sizes_5;
            std::vector<std::vector<std::vector<std::uint8_t>>> names_5;

            for (auto i = 0; i < snap_sizes.size(); i++) {
                std::vector<size_t> as_sizes_2;
                std::vector<std::vector<std::uint8_t>> names_2;

                std::vector<size_t> as_sizes_4;
                std::vector<std::vector<std::uint8_t>> names_4;
                for (auto k = 0; k < snap_sizes[i]; k++) {
                    std::vector<std::uint8_t> app_name(vm_snaps_list[i][k][0].begin(), vm_snaps_list[i][k][0].end());
                    as_sizes_2.push_back(app_name.size() * sizeof(decltype(app_name)::value_type));

                    std::vector<std::uint8_t> desc_name(vm_snaps_list[i][k][1].begin(), vm_snaps_list[i][k][1].end());
                    as_sizes_4.push_back(desc_name.size() * sizeof(decltype(desc_name)::value_type));

                    names_2.push_back(app_name);
                    names_4.push_back(desc_name);
                    shm.size += sizeof(shadow::shadow_app_status);
                    shm.size += as_sizes_2.back();
                    shm.size += as_sizes_4.back();
                }

                as_sizes_3.push_back(as_sizes_2);
                names_3.push_back(names_2);
                as_sizes_5.push_back(as_sizes_4);
                names_5.push_back(names_4);
            }

            std::cout << "total expected size: " << shm.size << std::endl;
            auto size_written = 0;

            // writing header
            boost::asio::async_write(
                *socket, boost::asio::buffer(reinterpret_cast<char*>(&shm), sizeof(shm)),
                std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

            // writing total applications
            boost::asio::async_write(
                *socket, boost::asio::buffer(reinterpret_cast<char*>(&sta), sizeof(sta)),
                std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

            size_written += sizeof(sta);
            std::cout << "size written: " << size_written << std::endl;

            for (auto i = 0; i < vm_list.size(); i++) {
                shadow::shadow_app_status sat;
                sat.status = static_cast<shadow::shadow_app_status::app_status>(vm_running_list[i]);
                sat.size = as_sizes[i];

                boost::asio::async_write(
                    *socket, boost::asio::buffer(reinterpret_cast<char*>(&sat), sizeof(sat)),
                    std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

                size_written += sizeof(sat);
                std::cout << "size written: " << size_written << std::endl;

                boost::asio::async_write(
                    *socket, boost::asio::buffer(reinterpret_cast<char*>(names[i].data()), as_sizes[i]),
                    std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

                size_written += as_sizes[i];
                std::cout << "size written: " << size_written << std::endl;
            }

            // writing total snaps
            boost::asio::async_write(
                *socket, boost::asio::buffer(reinterpret_cast<char*>(&stvms), sizeof(stvms)),
                std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

            size_written += sizeof(stvms);
            std::cout << "size written: " << size_written << std::endl;

            // writing total snaps sizes
            boost::asio::async_write(
                *socket, boost::asio::buffer(reinterpret_cast<char*>(snap_sizes.data()), stvms.size * sizeof(std::uint32_t)),
                std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

            size_written += stvms.size * sizeof(std::uint32_t);
            std::cout << "size written1: " << size_written << std::endl;

            for (auto i = 0; i < snap_sizes.size(); i++) {
                for (auto k = 0; k < snap_sizes[i]; k++) {
                    shadow::shadow_app_status sat;
                    sat.size = as_sizes_3[i][k];
                    sat.alt_size = as_sizes_5[i][k];

                    boost::asio::async_write(
                        *socket, boost::asio::buffer(reinterpret_cast<char*>(&sat), sizeof(sat)),
                        std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

                    size_written += sizeof(sat);
                    std::cout << "size written: " << size_written << std::endl;

                    boost::asio::async_write(
                        *socket, boost::asio::buffer(reinterpret_cast<char*>(names_3[i][k].data()), as_sizes_3[i][k]),
                        std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

                    size_written += as_sizes_3[i][k];
                    std::cout << "size written: " << size_written << std::endl;

                    boost::asio::async_write(
                        *socket, boost::asio::buffer(reinterpret_cast<char*>(names_5[i][k].data()), as_sizes_5[i][k]),
                        std::bind(&HostServer::HostServerClient::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

                    size_written += as_sizes_5[i][k];
                    std::cout << "size written: " << size_written << std::endl;
                }
            }

        } else {
            std::cerr << "host server client: " << client_number
                      << ", socket wasn't open while attempting to "
                         "write, closing socket"
                      << std::endl;
            reset();
        }
    } else
        std::cerr << "host server client: error vm list and vm_running list sizes didn't match, dropping update" << std::endl;
}