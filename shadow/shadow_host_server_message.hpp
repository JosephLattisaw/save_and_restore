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
        TAKE_CONTROL = 2,
        START_SIM = 3,
        STOP_SIM = 4,
        PLAY_SIMICS = 5,
        PAUSE_SIMICS = 6,
    } message;
};

struct shadow_main_status {
    std::uint8_t in_control = false;
    std::uint8_t simulation_started = false;
    std::uint8_t simics_playing = false;
    std::uint64_t config_number = 0;
};

struct shadow_total_apps {
    std::uint32_t total_applications = 0;
    std::uint32_t total_configurations = 0;
};

struct shadow_config_message {
    std::uint32_t configuration_number = 0;
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

using ApplicationStatusesEnum = std::vector<shadow_app_status::app_status>;
}  // namespace shadow

#endif