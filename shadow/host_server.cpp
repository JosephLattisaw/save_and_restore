#include "host_server.hpp"

#include <iostream>

#include "shadow_host_server_message.hpp"

HostServer::HostServer(boost::asio::io_service& io_service, std::uint16_t port,
                       std::vector<std::string> app_names, shadow::ApplicationStatuses app_statuses)
    : io_service(io_service),
      port(port),
      acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      application_names(app_names),
      application_statuses(app_statuses) {
    start_async_accept();
}

HostServer::~HostServer() {}

void HostServer::start_async_accept() {
    assert(!temp_socket);
    temp_socket = std::make_unique<boost::asio::ip::tcp::socket>(io_service);
    acceptor.async_accept(*temp_socket, [&](const boost::system::error_code& error) {
        if (!error) {
            std::cout << "HostServer: accepted connection on port " << port << std::endl;
            std::cout << "HostServer: assigned client id of: " << client_number_count << std::endl;
            auto c = std::make_unique<HostServerClient>(
                client_number_count, io_service, std::move(temp_socket),
                [&](std::uint64_t client_id) {
                    auto it = socket_map.find(client_id);
                    if (it != socket_map.end()) {
                        socket_map.erase(client_id);
                        std::cout << "HostServer: client " << client_id << " disconnected"
                                  << std::endl;
                    } else
                        std::cerr << "HostServer: could not find socket to erase" << std::endl;
                });

            socket_map.insert(std::make_pair(client_number_count, std::move(c)));

            // sanity check
            auto it = socket_map.find(client_number_count);
            assert(it != socket_map.end());

            it->second->send_application_update(application_names, application_statuses);

            client_number_count++;
        } else {
            std::cerr << "HostServer: attempted to accept connection on port " << port
                      << " but an error occurred: " << error.message() << std::endl;
            temp_socket.reset();
            return;
        }

        // starting to accept connections again now that our connection has been processed
        start_async_accept();
    });
}

HostServer::HostServerClient::HostServerClient(std::uint64_t client_number,
                                               boost::asio::io_service& io_service,
                                               std::unique_ptr<boost::asio::ip::tcp::socket> sock,
                                               Disconnect_Callback callback)
    : client_number(client_number),
      io_service(io_service),
      socket(std::move(sock)),
      disconnect_callback{callback} {
    start_read();
}

void HostServer::HostServerClient::start_read() {
    assert(socket);
    boost::asio::async_read(
        *socket, message_buffer, boost::asio::transfer_exactly(sizeof(shadow::shadow_host_message)),
        [&](const boost::system::error_code& error, std::size_t bytes_transferred) {
            if (!error) {
                if (message_buffer.size() == sizeof(shadow::shadow_host_message)) {
                    reset_buffers();
                    start_read();
                } else {
                    std::cerr << "host server client " << client_number
                              << ": received invalid message size of: " << bytes_transferred
                              << ", expected: " << sizeof(shadow::shadow_host_message) << std::endl;
                    reset();
                }
            } else {
                std::cerr << "host server client " << client_number
                          << ": encountered error when reading header : " << error.message()
                          << std::endl;
                reset();
            }
        });
}

void HostServer::HostServerClient::reset_buffers() {
    message_buffer.consume(message_buffer.size());
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

void HostServer::HostServerClient::send_application_update(
    std::vector<std::string> application_names, shadow::ApplicationStatuses application_statuses) {
    assert(application_names.size() == application_statuses.size());
    assert(socket);
    if (socket && socket->is_open()) {
        // creating main header
        shadow::shadow_host_message shm;
        shm.message = shadow::shadow_host_message::host_message::APP_STATUS;

        // total applications
        shadow::shadow_total_apps sta;
        sta.total_applications = application_statuses.size();
        shm.size += sizeof(shadow::shadow_total_apps);

        std::vector<size_t> as_sizes;
        std::vector<std::vector<std::uint8_t>> names;

        for (auto i = 0; i < application_names.size(); i++) {
            std::vector<std::uint8_t> app_name(application_names[i].begin(),
                                               application_names[i].end());

            as_sizes.push_back(app_name.size() * sizeof(decltype(app_name)::value_type));
            names.push_back(app_name);

            shm.size += sizeof(shadow::shadow_app_status);
            shm.size += as_sizes.back();
        }

        // writing header
        boost::asio::async_write(*socket,
                                 boost::asio::buffer(reinterpret_cast<char*>(&shm), sizeof(shm)),
                                 std::bind(&HostServer::HostServerClient::write_error_handler, this,
                                           std::placeholders::_1, std::placeholders::_2));

        // writing total applications
        boost::asio::async_write(*socket,
                                 boost::asio::buffer(reinterpret_cast<char*>(&sta), sizeof(sta)),
                                 std::bind(&HostServer::HostServerClient::write_error_handler, this,
                                           std::placeholders::_1, std::placeholders::_2));

        for (auto i = 0; i < application_names.size(); i++) {
            shadow::shadow_app_status sha;
            sha.status = application_statuses[i];
            sha.size = as_sizes[i];

            boost::asio::async_write(
                *socket, boost::asio::buffer(reinterpret_cast<char*>(&sha), sizeof(sha)),
                std::bind(&HostServer::HostServerClient::write_error_handler, this,
                          std::placeholders::_1, std::placeholders::_2));
        }
    } else {
        std::cerr << "host server client: " << client_number
                  << ", socket wasn't open while attempting to "
                     "write, closing socket"
                  << std::endl;
        reset();
    }
}

void HostServer::HostServerClient::write_error_handler(const boost::system::error_code& error,
                                                       std::size_t bytes_transferred) {
    if (error) {
        std::cerr << "host server client: " << client_number
                  << " error writing data: " << error.message() << std::endl;
        reset();
    }
}