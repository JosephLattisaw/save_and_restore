#include "client.hpp"

#include <iostream>

Client::Client(boost::asio::io_service &io_service, AppInitialStatusCallback ais_cb)
    : io_service(io_service),
      socket(io_service),
      endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 20001),
      app_initial_status_callback{ais_cb} {
    start_async_connect();
}

void Client::start_async_connect() {
    socket.async_connect(endpoint, [&](const boost::system::error_code &error) {
        if (!error) {
            std::cout << "client: connected successfully" << std::endl;
            update_connection_status(true);

            start_read();
        } else {
            std::cerr << "client: connection failed" << std::endl;
            start_async_connect();  // TODO don't want to do this, want to do timer instead
        }
    });
}

void Client::start_read() {
    boost::asio::async_read(
        socket, header_buffer, boost::asio::transfer_exactly(sizeof(shadow::shadow_host_message)),
        [&](const boost::system::error_code &error, std::size_t bytes_transferred) {
            if (!error && socket.is_open()) {
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
                                    const std::uint8_t *data = boost::asio::buffer_cast<const std::uint8_t *>(data_buffer.data());

                                    switch (msg) {
                                        case shadow::shadow_host_message::host_message::APP_INITIAL_STATUS: {
                                            std::cout << "client: received app status message" << std::endl;

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
                                            }
                                        } break;
                                        case shadow::shadow_host_message::host_message::MAIN_STATUS:
                                            std::cout << "client: received main status message" << std::endl;
                                            break;
                                    }

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

void Client::update_connection_status(bool status) {
    // TODO
}

void Client::reset() {
    std::cout << "client: resetting socket" << std::endl;
    try {
        socket.close();
    } catch (const boost::exception &) {
        std::cerr << "client: error closing socket connection" << std::endl;
    }

    reset_buffers();
    update_connection_status(false);
}

void Client::reset_buffers() {
    header_buffer.consume(header_buffer.size());
    data_buffer.consume(data_buffer.size());
}