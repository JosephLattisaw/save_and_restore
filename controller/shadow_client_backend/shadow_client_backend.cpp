#include <boost/asio.hpp>
#include <iostream>

#include "client.hpp"

// dart api headers
#include "include/dart_api.h"
#include "include/dart_api_dl.h"
#include "include/dart_native_api.h"

namespace {
// These are needed for the actual backend
// Creating an actual object within frontend is overtly complicated and still being developed within
// the language itself. No one should be accessing these outside of the library itself
boost::asio::io_service io_service;
std::shared_ptr<Client> client;

static void FreeFinalizer(void *, void *value) {
    std::cout << "FreeFinalizer called: " << value << std::endl;
    std::cout << std::flush;
    fflush(stdout);
    free(value);
}

static void post_data_object(std::int64_t port, std::vector<std::uint8_t> data) {
    void *request_buffer = malloc(sizeof(uint8_t) * data.size());
    const size_t request_length = sizeof(uint8_t) * data.size();
    std::copy(data.begin(), data.end(), reinterpret_cast<std::uint8_t *>(request_buffer));

    Dart_CObject dart_object;
    dart_object.type = Dart_CObject_kExternalTypedData;
    dart_object.value.as_external_typed_data.type = Dart_TypedData_kUint8;
    dart_object.value.as_external_typed_data.length = request_length;
    dart_object.value.as_external_typed_data.data = reinterpret_cast<std::uint8_t *>(request_buffer);
    dart_object.value.as_external_typed_data.peer = request_buffer;
    dart_object.value.as_external_typed_data.callback = FreeFinalizer;

    std::cout << "posting application size statuses to port: " << port << std::endl;
    Dart_PostCObject_DL(port, &dart_object);
}

static void post_data_string(std::int64_t port, std::string str) {
    std::cout << "posting name: " << str << std::endl;

    Dart_CObject dart_object;
    dart_object.type = Dart_CObject_kString;
    dart_object.value.as_string = &str[0];
    Dart_PostCObject_DL(port, &dart_object);
}

static void post_data_bool(std::int64_t port, bool value) {
    std::cout << "posting bool: " << value << std::endl;

    Dart_CObject dart_object;
    dart_object.type = Dart_CObject_kBool;
    dart_object.value.as_bool = value;
    Dart_PostCObject_DL(port, &dart_object);
}
}  // namespace

extern "C" {
void create_client(bool using_dart = false, std::int64_t initial_status_port = 0, std::int64_t initial_names_port = 0,
                   std::int64_t connection_port = 0, std::int64_t control_status_port = 0, std::int64_t sim_status_port = 0,
                   std::int64_t simics_status_port = 0) {
    if (!client)
        client = std::make_shared<Client>(
            io_service,
            [&, using_dart, initial_status_port, initial_names_port](std::vector<std::string> names, shadow::ApplicationStatuses statuses) {
                std::cout << "c_api: got application statuses of size: " << statuses.size() << std::endl;
                if (using_dart) {
                    post_data_object(initial_status_port, std::vector<std::uint8_t>(statuses.begin(), statuses.end()));
                    for (auto i : names) post_data_string(initial_names_port, i);
                }
            },
            [&, using_dart, connection_port](bool connection_status) {
                if (using_dart) post_data_bool(connection_port, connection_status);
            },
            [&, using_dart, control_status_port](bool control_status) {
                if (using_dart) post_data_bool(control_status_port, control_status);
            },
            [&, using_dart, sim_status_port](bool sim_status) {
                // TODO
                if (using_dart) post_data_bool(sim_status_port, sim_status);
            },
            [&, using_dart, simics_status_port](bool simics_status) {
                if (using_dart) post_data_bool(simics_status_port, simics_status);
            });
}

// This destroys our client object
// TODO find out if this is actually needed since we are using smart pointers
void destroy_client() {
    std::cout << "destroy client" << std::endl;
    client.reset();
}

// This runs are io service
// It's important we are doing this from a different thread.
// Boost's IO service is thread safe
// TODO we need to make sure our application is completely thread safe
void run_service() {
    std::thread t([&] { io_service.run(); });
    t.detach();
}

void start_simulation(std::uint32_t configuration) {
    if (client) io_service.post(std::bind(&Client::start_simulation, client, configuration));
}

void stop_simulation() {
    if (client) io_service.post(std::bind(&Client::stop_simulation, client));
}

void start_simics() {
    if (client) io_service.post(std::bind(&Client::start_simics, client));
}

void pause_simics() {
    if (client) io_service.post(std::bind(&Client::pause_simics, client));
}

void take_control() {
    std::cout << "taking contro;() " << std::endl;
    if (client) io_service.post(std::bind(&Client::take_control, client));
}

DART_EXPORT intptr_t InitializeDartApi(void *data) { return Dart_InitializeApiDL(data); }
}

int main(int argc, char **argv) {
    create_client();
    run_service();
    io_service.run();
    return 0;
}