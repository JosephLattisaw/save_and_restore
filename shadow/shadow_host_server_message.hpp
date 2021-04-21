#ifndef SHADOW_HOST_SERVER_MESSAGE_HPP
#define SHADOW_HOST_SERVER_MESSAGE_HPP

namespace shadow {
typedef std::uint8_t app_status_t;
using ApplicationStatuses = std::vector<app_status_t>;

struct shadow_host_message {
    std::uint32_t size = 0;  // size of all following packets

    enum struct host_message : std::uint32_t {
        MAIN_STATUS = 0,
        APP_INITIAL_STATUS = 1,
    } message;
};

struct shadow_main_status {
    std::uint8_t in_control = false;
    std::uint8_t simulation_started = false;
    std::uint8_t simics_playing = false;
};

struct shadow_total_apps {
    std::uint32_t total_applications;
};

struct shadow_app_status {
    std::uint32_t size = 0;  // size of following application

    enum struct app_status : std::uint8_t {
        NOT_RUNNING = 0,
        RUNNING = 1,
        CRASHED = 2,
    } status;

    std::uint8_t* application_name;
};
}  // namespace shadow

#endif