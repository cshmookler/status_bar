// Standard includes
#include <string>

// Local includes
#include "../constants.hpp"

namespace sbar {

// class readonly_file {
//     std::FILE* file_;

//   public:
//     explicit readonly_file(const char* path) : file_(std::fopen(path,
//     "r")) {
//     }

//     readonly_file(const readonly_file&) = delete;
//     readonly_file(readonly_file&&) noexcept = default;
//     readonly_file& operator=(const readonly_file&) = delete;
//     readonly_file& operator=(readonly_file&&) noexcept = default;

//     ~readonly_file() {
//         std::fclose(this->file_);
//         // do nothing if the file fails to close.
//     }

//     [[nodiscard]] bool good() const {
//         return this->file_ == nullptr;
//     }

//     template<typename... request_t>
//     bool request(unsigned long request_type, request_t&... request) {
//         return ::sbar::request(fileno(this->file_), request_type,
//         request...);
//     }
// };

std::string get_camera_status() {
    // readonly_file file{ "/dev/video0" };

    // struct v4l2_capability capabilities {};

    // const int argp = 0;

    // struct v4l2_requestbuffers buffers {};

    // struct v4l2_queryctrl ctrl {};

    // if (! file.request(VIDIOC_QUERYCTRL, ctrl)) {
    //     return sbar::error_str;
    // }

    // // return sprintf("%s", capabilities.driver);
    // return sprintf("%s", ctrl.name);

    return sbar::null_str;
}

} // namespace sbar
