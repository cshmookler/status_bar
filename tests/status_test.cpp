// Standard includes
#include <string>

// External includes
#include <gtest/gtest.h>

// Local includes
#include "../src/status.hpp"

TEST(status, time) {
    sbar::Fields fields;
    std::string time = fields.get_time();
    EXPECT_STRNE(time.data(), sbar::error_str);
}

TEST(status, uptime) {
    sbar::Fields fields;
    std::string uptime = fields.get_uptime();
    EXPECT_STRNE(uptime.data(), sbar::error_str);
}

TEST(status, disk) {
    sbar::Fields fields;
    std::string disk_percent = fields.get_disk_percent();
    EXPECT_STRNE(disk_percent.data(), sbar::error_str);
}

TEST(status, memory) {
    sbar::Fields fields;
    std::string memory = fields.get_memory_percent();
    EXPECT_STRNE(memory.data(), sbar::error_str);
}

TEST(status, swap) {
    sbar::Fields fields;
    std::string swap = fields.get_swap_percent();
    EXPECT_STRNE(swap.data(), sbar::error_str);
}

TEST(status, cpu) {
    sbar::Fields fields;
    std::string percent = fields.get_cpu_percent();
    EXPECT_STREQ(percent.data(), sbar::standby_str);
    percent = fields.get_cpu_percent();
    EXPECT_STRNE(percent.data(), sbar::standby_str);
    EXPECT_STRNE(percent.data(), sbar::error_str);
}

TEST(status, cpuTemp) {
    sbar::Fields fields;
    std::string temp = fields.get_cpu_temperature();
    EXPECT_STRNE(temp.data(), sbar::error_str);
}

TEST(status, load1) {
    sbar::Fields fields;
    std::string load1 = fields.get_one_minute_load_average();
    EXPECT_STRNE(load1.data(), sbar::error_str);
}

TEST(status, load5) {
    sbar::Fields fields;
    std::string load5 = fields.get_five_minute_load_average();
    EXPECT_STRNE(load5.data(), sbar::error_str);
}

TEST(status, load15) {
    sbar::Fields fields;
    std::string load15 = fields.get_fifteen_minute_load_average();
    EXPECT_STRNE(load15.data(), sbar::error_str);
}

TEST(status, batteryStatus) {
    sbar::Fields fields;
    std::string status = fields.get_battery_status();
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, batteryDevice) {
    sbar::Fields fields;
    std::string device = fields.get_battery_device();
    EXPECT_STRNE(device.data(), sbar::error_str);
}

TEST(status, battery) {
    sbar::Fields fields;
    std::string percent = fields.get_battery_percent();
    EXPECT_STRNE(percent.data(), sbar::error_str);
}

TEST(status, batteryTimeRemaining) {
    sbar::Fields fields;
    std::string remaining = fields.get_battery_time_remaining();
    EXPECT_STREQ(remaining.data(), sbar::standby_str);
}

TEST(status, backlight) {
    sbar::Fields fields;
    std::string percent = fields.get_backlight_percent();
    EXPECT_STRNE(percent.data(), sbar::error_str);
}

TEST(status, networkStatus) {
    sbar::Fields fields;
    std::string status = fields.get_network_status();
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, networkDevice) {
    sbar::Fields fields;
    std::string device = fields.get_network_device();
    EXPECT_STRNE(device.data(), sbar::error_str);
}

TEST(status, networkSSID) {
    sbar::Fields fields;
    std::string ssid = fields.get_network_ssid();
    EXPECT_STRNE(ssid.data(), sbar::error_str);
}

TEST(status, networkStrength) {
    sbar::Fields fields;
    std::string signal = fields.get_network_signal_strength_percent();
    EXPECT_STRNE(signal.data(), sbar::error_str);
}

TEST(status, networkUpload) {
    sbar::Fields fields;
    std::string upload = fields.get_network_upload();
    EXPECT_STREQ(upload.data(), sbar::standby_str);
    upload = fields.get_network_upload();
    EXPECT_STRNE(upload.data(), sbar::error_str);
}

TEST(status, networkDownload) {
    sbar::Fields fields;
    std::string download = fields.get_network_download();
    EXPECT_STREQ(download.data(), sbar::standby_str);
    download = fields.get_network_download();
    EXPECT_STRNE(download.data(), sbar::error_str);
}

TEST(status, volumeStatus) {
    sbar::Fields fields;
    std::string status = fields.get_volume_status();
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, volume) {
    sbar::Fields fields;
    std::string volume = fields.get_volume_percent();
    EXPECT_STRNE(volume.data(), sbar::error_str);
}

TEST(status, captureStatus) {
    sbar::Fields fields;
    std::string status = fields.get_capture_status();
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, capture) {
    sbar::Fields fields;
    std::string capture = fields.get_capture_percent();
    EXPECT_STRNE(capture.data(), sbar::error_str);
}

TEST(status, microphone) {
    sbar::Fields fields;
    std::string microphone = fields.get_microphone_status();
    EXPECT_STRNE(microphone.data(), sbar::error_str);
}

TEST(status, camera) {
    sbar::Fields fields;
    std::string status = fields.get_camera_status();
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, user) {
    sbar::Fields fields;
    std::string user = fields.get_user();
    EXPECT_STRNE(user.data(), sbar::error_str);
}

TEST(status, kernelStatus) {
    sbar::Fields fields;
    std::string kernel_status = fields.get_outdated_kernel_indicator();
    EXPECT_STRNE(kernel_status.data(), sbar::error_str);
}
