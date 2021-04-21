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
std::unique_ptr<Client> client;

static void FreeFinalizer(void *, void *value) {
    std::cout << "FreeFinalizer called: " << value << std::endl;
    std::cout << std::flush;
    fflush(stdout);
    free(value);
}

static void post_data_object(int port, std::vector<std::uint8_t> data) {
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
    Dart_PostCObject_DL(port, &dart_object);
}
}  // namespace

extern "C" {
void create_client(int initial_status_port) {
    if (!client)
        client = std::make_unique<Client>(io_service, [&, initial_status_port](std::vector<std::string> names, shadow::ApplicationStatuses statuses) {
            post_data_object(initial_status_port, std::vector<std::uint8_t>(statuses.begin(), statuses.end()));
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

DART_EXPORT intptr_t InitializeDartApi(void *data) { return Dart_InitializeApiDL(data); }
}

int main(int argc, char **argv) {
    create_client(-1);
    run_service();
    io_service.run();
    return 0;
}