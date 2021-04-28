#include "client.hpp"

#include <iostream>

Client::Client(boost::asio::io_service &io_service, AppInitialStatusCallback ais_cb, ConnectionCallback conn_cb, ControlStatusCallback cs_cb,
               SimulationStatusCallback ss_cb, SimicsStatusCallback s_cb, VMListCallback vl_cb, VMRunningCallback vr_cb)
    : io_service(io_service),
      socket(io_service),
      endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 20001),
      app_initial_status_callback{ais_cb},
      connection_callback{conn_cb},
      control_status_callback{cs_cb},
      simulation_status_callback{ss_cb},
      simics_status_callback{s_cb},
      vm_list_callback{vl_cb},
      vm_running_callback{vr_cb},
      timer(io_service) {
    start_async_connect();
}

void Client::start_async_connect() {
    attempting_connection = true;  // prevents this from getting called multiple times
    socket.async_connect(endpoint, [&](const boost::system::error_code &error) {
        attempting_connection = false;
        if (!error) {
            std::cout << "client: connected successfully" << std::endl;
            connected = true;
            connection_callback(true);

            start_read();
        } else {
            std::cerr << "client: connection failed" << std::endl;
            start_connect();
        }
    });
}

void Client::start_connect() {
    if (!timer_fired && !attempting_connection && !connected) {  // prevents timer from getting called multiple times
        timer_fired = true;
        timer.expires_after(std::chrono::seconds(1));
        timer.async_wait([&](const boost::system::error_code &error) {
            timer_fired = false;
            if (error != boost::asio::error::operation_aborted) {
                std::cerr << "client: encountered error while processeing start_connect" << std::endl;
                if (!connected) start_async_connect();  // try again?
            } else if (error) {
                start_connect();
            }
        });
    }
}

void Client::start_read() {
    boost::asio::async_read(
        socket, header_buffer, boost::asio::transfer_exactly(sizeof(shadow::shadow_host_message)),
        [&](const boost::system::error_code &error, std::size_t bytes_transferred) {
            if (!error && connected) {
                if (header_buffer.size() == sizeof(shadow::shadow_host_message)) {
                    const shadow::shadow_host_message *shm = boost::asio::buffer_cast<const shadow::shadow_host_message *>(header_buffer.data());

                    std::cout << "client: finished reading header" << std::endl;
                    std::cout << "client: attempting to read " << shm->size << " bytes" << std::endl;

                    auto msg = shm->message;
                    auto msg_size = shm->size;

                    boost::asio::async_read(
                        socket, data_buffer, boost::asio::transfer_exactly(msg_size),
                        [&, msg, msg_size](const boost::system::error_code &error, std::size_t bytes_transferred) {
                            if (!error) {
                                if (data_buffer.size() == msg_size) {
                                    std::cout << "client received the correct amount of data" << std::endl;
                                    std::cout << "client msg: " << static_cast<int>(msg) << std::endl;
                                    std::cout << "client msg size: " << msg_size << std::endl;

                                    switch (msg) {
                                        case shadow::shadow_host_message::host_message::APP_INITIAL_STATUS: {
                                            std::cout << "client: received app status message" << std::endl;
                                            const std::uint8_t *data = boost::asio::buffer_cast<const std::uint8_t *>(data_buffer.data());

                                            const shadow::shadow_total_apps *sta = reinterpret_cast<const shadow::shadow_total_apps *>(&data[0]);

                                            std::cout << "client: total applications: " << sta->total_applications << std::endl;

                                            auto it = sizeof(shadow::shadow_total_apps);
                                            std::vector<std::string> apps;
                                            shadow::ApplicationStatuses app_statuses;
                                            for (auto i = 0; i < sta->total_applications; i++) {
                                                const shadow::shadow_app_status *sat = reinterpret_cast<const shadow::shadow_app_status *>(&data[it]);

                                                std::cout << "client: application name size: " << sat->size << std::endl;
                                                std::cout << "client: application status: " << static_cast<int>(sat->status) << std::endl;

                                                app_statuses.push_back(static_cast<shadow::app_status_t>(sat->status));

                                                it += sizeof(shadow::shadow_app_status);

                                                std::string name(&data[it], &data[it] + sat->size);
                                                apps.push_back(name);
                                                std::cout << "client: application name: " << name << std::endl;
                                                it += sat->size;
                                            }

                                            if (apps.size() == app_statuses.size()) {
                                                std::cout << "client: received valid application "
                                                             "data update"
                                                          << std::endl;
                                                application_names = apps;
                                                application_statuses = app_statuses;
                                                app_initial_status_callback(application_names, application_statuses);
                                            } else {
                                                std::cerr << "client: app size doesn't equal app statuses size?" << std::endl;
                                                reset();
                                                return;
                                            }
                                        } break;
                                        case shadow::shadow_host_message::host_message::VM_LIST: {
                                            std::cout << "client: received vm list message" << std::endl;
                                            const std::uint8_t *data = boost::asio::buffer_cast<const std::uint8_t *>(data_buffer.data());

                                            const shadow::shadow_total_apps *sta = reinterpret_cast<const shadow::shadow_total_apps *>(&data[0]);

                                            std::cout << "client: total virtual machines: " << sta->total_applications << std::endl;

                                            auto it = sizeof(shadow::shadow_total_apps);
                                            std::cout << "size read: " << it << std::endl;

                                            std::vector<std::string> vms;
                                            shadow::ApplicationStatuses app_statuses;
                                            for (auto i = 0; i < sta->total_applications; i++) {
                                                const shadow::shadow_app_status *sat = reinterpret_cast<const shadow::shadow_app_status *>(&data[it]);

                                                std::cout << "client: virtual machine name size: " << sat->size << std::endl;
                                                std::cout << "client: virtual machine status: " << static_cast<int>(sat->status) << std::endl;

                                                app_statuses.push_back(static_cast<shadow::app_status_t>(sat->status));

                                                it += sizeof(shadow::shadow_app_status);
                                                std::cout << "size read: " << it << std::endl;

                                                std::string name(&data[it], &data[it] + sat->size);
                                                vms.push_back(name);
                                                std::cout << "client: vm name: " << name << std::endl;
                                                it += sat->size;
                                                std::cout << "size read: " << it << std::endl;
                                            }

                                            std::cout << "it1: " << it << " ms: " << msg_size << std::endl;

                                            // reading total vm snapshots:
                                            const shadow::shadow_total_vm_snapshots *stvms =
                                                reinterpret_cast<const shadow::shadow_total_vm_snapshots *>(&data[it]);
                                            std::cout << "total vms: with snapshots: " << stvms->size << std::endl;
                                            it += sizeof(shadow::shadow_total_vm_snapshots);
                                            std::cout << "it1: " << it << " ms: " << msg_size << std::endl;
                                            std::cout << "size read: " << it << std::endl;

                                            // getting each machines total vm snapshots
                                            const std::uint32_t *st_data_sz = reinterpret_cast<const std::uint32_t *>(&data[it]);

                                            it += sizeof(std::uint32_t) * stvms->size;
                                            std::cout << "size read: " << it << std::endl;

                                            std::vector<std::vector<std::string>> snap_names;
                                            std::vector<std::vector<std::string>> snap_desc;

                                            for (auto i = 0; i < stvms->size; i++) {
                                                std::cout << "vm snap size: " << st_data_sz[i] << std::endl;
                                                std::vector<std::string> sn_names;
                                                std::vector<std::string> sn_descs;

                                                for (auto k = 0; k < st_data_sz[i]; k++) {
                                                    std::cout << "k: " << k << std::endl;
                                                    const shadow::shadow_app_status *sas =
                                                        reinterpret_cast<const shadow::shadow_app_status *>(&data[it]);
                                                    it += sizeof(shadow::shadow_app_status);
                                                    std::cout << "size read: " << it << std::endl;

                                                    std::string name(&data[it], &data[it] + sas->size);
                                                    std::cout << "client: snapshot name: " << name << std::endl;
                                                    it += sas->size;

                                                    std::string desc(&data[it], &data[it] + sas->alt_size);
                                                    std::cout << "client: snapshot desc: " << name << std::endl;
                                                    it += sas->alt_size;

                                                    std::cout << "size read: " << it << std::endl;
                                                    sn_names.push_back(name);
                                                    sn_descs.push_back(desc);
                                                }
                                                snap_names.push_back(sn_names);
                                                snap_desc.push_back(sn_descs);
                                            }

                                            if (vms.size() == app_statuses.size() && vms.size() == snap_names.size() &&
                                                snap_names.size() == snap_desc.size()) {
                                                // std::cout << "client: received valid vm data update" << std::endl;
                                                vm_list = vms;
                                                vm_running_list = app_statuses;
                                                vm_list_callback(vm_list, vm_running_list, snap_names, snap_desc);
                                            } else {
                                                std::cerr << "client: app size doesn't equal app statuses size?" << std::endl;
                                                reset();
                                                return;
                                            }
                                        } break;
                                        case shadow::shadow_host_message::host_message::MAIN_STATUS: {
                                            std::cout << "client: received main status message" << std::endl;
                                            const shadow::shadow_main_status *shs =
                                                boost::asio::buffer_cast<const shadow::shadow_main_status *>(data_buffer.data());

                                            control_status_callback(shs->in_control);
                                            simulation_status_callback(shs->simulation_started);
                                            simics_status_callback(shs->simics_playing);
                                            vm_running_callback(shs->vm_running);
                                        } break;
                                        default:
                                            std::cerr << "client: error unknown message: " << static_cast<int>(msg) << std::endl;
                                            reset();
                                            return;
                                    }
                                    reset_buffers();
                                    start_read();

                                } else {
                                    std::cerr << "client: received invalid data size of: " << bytes_transferred << ", expected: " << msg_size
                                              << std::endl;
                                    reset();
                                }
                            } else {
                                std::cerr << "client: encountered error when reading data: " << error.message() << std::endl;
                                reset();
                            }
                        });

                } else {
                    std::cerr << "client: received invalid header size of: " << bytes_transferred
                              << ", expected: " << sizeof(shadow::shadow_host_message) << std::endl;
                    reset();
                }
            } else {
                std::cerr << "client: encountered error when reading header: " << error.message() << std::endl;
                reset();
            }
        });
}

void Client::reset() {
    std::cout << "client: resetting socket" << std::endl;
    try {
        socket.close();
    } catch (const boost::exception &) {
        std::cerr << "client: error closing socket connection" << std::endl;
    }

    reset_buffers();

    connection_callback(false);
    control_status_callback(false);
    simulation_status_callback(false);
    simics_status_callback(false);

    connected = false;
    start_connect();
}

void Client::reset_buffers() {
    header_buffer.consume(header_buffer.size());
    data_buffer.consume(data_buffer.size());
}

void Client::start_simulation(std::uint32_t configuration) {
    if (socket.is_open() && connected) {
        shadow::shadow_host_message shm;
        shm.size = sizeof(shadow::shadow_config_message);
        shm.message = shadow::shadow_host_message::host_message::START_SIM;
        boost::asio::async_write(socket, boost::asio::buffer(reinterpret_cast<char *>(&shm), sizeof(shm)),
                                 std::bind(&Client::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));

        shadow::shadow_config_message scm;
        scm.configuration_number = configuration;

        boost::asio::async_write(socket, boost::asio::buffer(reinterpret_cast<char *>(&scm), sizeof(scm)),
                                 std::bind(&Client::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void Client::stop_simulation() {
    if (socket.is_open() && connected) {
        shadow::shadow_host_message shm;
        shm.size = 0;
        shm.message = shadow::shadow_host_message::host_message::STOP_SIM;
        boost::asio::async_write(socket, boost::asio::buffer(reinterpret_cast<char *>(&shm), sizeof(shm)),
                                 std::bind(&Client::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void Client::start_simics() {
    if (socket.is_open() && connected) {
        shadow::shadow_host_message shm;
        shm.size = 0;
        shm.message = shadow::shadow_host_message::host_message::PLAY_SIMICS;
        boost::asio::async_write(socket, boost::asio::buffer(reinterpret_cast<char *>(&shm), sizeof(shm)),
                                 std::bind(&Client::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void Client::pause_simics() {
    if (socket.is_open() && connected) {
        shadow::shadow_host_message shm;
        shm.size = 0;
        shm.message = shadow::shadow_host_message::host_message::PAUSE_SIMICS;
        boost::asio::async_write(socket, boost::asio::buffer(reinterpret_cast<char *>(&shm), sizeof(shm)),
                                 std::bind(&Client::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void Client::take_control() {
    if (socket.is_open() && connected) {
        shadow::shadow_host_message shm;
        shm.size = 0;
        shm.message = shadow::shadow_host_message::host_message::TAKE_CONTROL;
        boost::asio::async_write(socket, boost::asio::buffer(reinterpret_cast<char *>(&shm), sizeof(shm)),
                                 std::bind(&Client::write_error_handler, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void Client::write_error_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {
    if (error) {
        std::cerr << "client: error writing data: " << error.message() << std::endl;
        reset();
    }
}