// Standard includes
#include <cstring>
#include <string>

// External includes
#include <linux/wireless.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

// Local includes
#include "../status.hpp"

namespace sbar {

Network::Network() {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const networks_path = "/sys/class/net/";
    const char* const network_operstate_filename = "operstate";
    const char* const network_device_path = "device/";
    const char* const network_statistics_path = "statistics/";
    const char* const network_statistics_rx_bytes_filename = "rx_bytes";
    const char* const network_statistics_tx_bytes_filename = "tx_bytes";

    for (const std::filesystem::directory_entry& device :
      std::filesystem::directory_iterator(networks_path)) {
        if (! std::filesystem::exists(
              device.path() / network_operstate_filename)) {
            continue;
        }
        if (! std::filesystem::exists(device.path() / network_device_path)) {
            continue;
        }
        if (! std::filesystem::exists(device.path() / network_statistics_path
              / network_statistics_rx_bytes_filename)) {
            continue;
        }
        if (! std::filesystem::exists(device.path() / network_statistics_path
              / network_statistics_tx_bytes_filename)) {
            continue;
        }

        this->path_ = device.path();
        this->good_ = true;
        return;
    }
}

enum class Network_state : size_t {
    error,
    up,
    dormant,
    down,
};

Network_state get_network_state(const Network& network) {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const network_operstate_filename = "operstate";
    // const char* const network_operstate_unknown = "unknown";
    const char* const network_operstate_up = "up";
    const char* const network_operstate_dormant = "dormant";
    const char* const network_operstate_down = "down";

    std::string operstate =
      get_first_line(network.path() / network_operstate_filename);
    if (operstate == sbar::null_str) {
        return Network_state::error;
    }

    if (operstate == network_operstate_up) {
        return Network_state::up;
    }
    if (operstate == network_operstate_dormant) {
        return Network_state::dormant;
    }
    if (operstate == network_operstate_down) {
        return Network_state::down;
    }

    return Network_state::error;
}

std::string get_network_status(const Network& network) {
    switch (get_network_state(network)) {
        case Network_state::up:
            return "🟢";
        case Network_state::dormant:
            return "🟡";
        case Network_state::down:
            return "🔴";
        case Network_state::error:
            /* fallthrough */
        default:
            return sbar::error_str;
    }
}

std::string get_network_device(const Network& network) {
    return network->stem();
}

template<typename... Request_t>
bool request(
  int file_descriptor, unsigned long request_type, Request_t&... request) {
    auto result = ioctl(file_descriptor, request_type, &request...);
    int err = errno;
    if (result < 0) {
        std::cerr << "ioctl(" << request_type << "): " << std::strerror(err)
                  << '\n';
    }
    return result >= 0;
}

class Unix_socket {
    static const int default_protocol = 0;

    int socket_file_descriptor_;

  public:
    Unix_socket(int domain, int type, int protocol = default_protocol)
    : socket_file_descriptor_(socket(domain, type, protocol)) {
    }

    Unix_socket(const Unix_socket&) = delete;
    Unix_socket(Unix_socket&&) noexcept = default;
    Unix_socket& operator=(const Unix_socket&) = delete;
    Unix_socket& operator=(Unix_socket&&) noexcept = default;

    ~Unix_socket() {
        close(this->socket_file_descriptor_);
        // do nothing if the socket fails to close.
    }

    [[nodiscard]] bool good() const {
        return this->socket_file_descriptor_ >= 0;
    }

    template<typename... Request_t>
    bool request(unsigned long request_type, Request_t&... request) {
        return ::sbar::request(
          this->socket_file_descriptor_, request_type, request...);
    }
};

std::string get_network_ssid(const Network& network) {
    // documentation:
    // https://github.com/torvalds/linux/blob/master/include/uapi/linux/wireless.h

    if (get_network_state(network) != Network_state::up) {
        return sbar::standby_str;
    }

    Unix_socket socket{ AF_INET, SOCK_DGRAM };
    if (! socket.good()) {
        return sbar::error_str;
    }

    iwreq iwreq_info{};

    std::strcpy(iwreq_info.ifr_ifrn.ifrn_name, network->stem().string().data());

    // This array must be 1 unit larger than the maximum ESSID size and default
    // initialized so that the ESSID is null-terminated.
    std::array<char, IW_ESSID_MAX_SIZE + 1> essid{};

    iwreq_info.u.essid.pointer = essid.data();
    iwreq_info.u.essid.length = essid.size();

    if (! socket.request(SIOCGIWESSID, iwreq_info)) {
        return sbar::error_str;
    }

    return std::string{ essid.data() };
}

std::string get_network_signal_strength_percent(const Network& network) {
    // documentation:
    // https://github.com/torvalds/linux/blob/master/include/uapi/linux/wireless.h

    if (get_network_state(network) != Network_state::up) {
        return sbar::standby_str;
    }

    Unix_socket socket{ AF_INET, SOCK_DGRAM };
    if (! socket.good()) {
        return sbar::error_str;
    }

    iwreq iwreq_info{};
    iw_statistics iw_statistics_info{};
    iwreq_info.u.data.pointer = &iw_statistics_info;
    iwreq_info.u.data.length = sizeof(iw_statistics_info);

    std::strcpy(iwreq_info.ifr_ifrn.ifrn_name, network->stem().string().data());

    if (! socket.request(SIOCGIWSTATS, iwreq_info)) {
        return sbar::error_str;
    }

    const double max_signal_strength = 70;
    double signal_strength =
      iw_statistics_info.qual.qual / max_signal_strength * 1e2;

    return sprintf("%.0f", signal_strength);
}

std::string get_network_upload(
  const Network& network, Network_data_stats& network_state_info) {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const network_statistics_path = "statistics/";
    const char* const network_statistics_tx_bytes_filename = "tx_bytes";

    std::string upload_bytes = get_first_line(network.path()
      / network_statistics_path / network_statistics_tx_bytes_filename);
    if (upload_bytes == sbar::null_str) {
        return sbar::error_str;
    }

    size_t upload_bytes_numeric = std::stoull(upload_bytes);

    auto upload_byte_difference =
      network_state_info.get_upload_byte_difference(upload_bytes_numeric);
    if (upload_byte_difference == upload_bytes_numeric) {
        return sbar::standby_str;
    }

    return sprintf("%i", upload_byte_difference);
}

std::string get_network_download(
  const Network& network, Network_data_stats& network_state_info) {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const network_statistics_path = "statistics/";
    const char* const network_statistics_rx_bytes_filename = "rx_bytes";

    std::string download_bytes = get_first_line(network.path()
      / network_statistics_path / network_statistics_rx_bytes_filename);
    if (download_bytes == sbar::null_str) {
        return sbar::error_str;
    }

    size_t download_bytes_numeric = std::stoull(download_bytes);

    auto download_byte_difference =
      network_state_info.get_download_byte_difference(download_bytes_numeric);
    if (download_byte_difference == download_bytes_numeric) {
        return sbar::standby_str;
    }

    return sprintf("%i", download_byte_difference);
}

} // namespace sbar
