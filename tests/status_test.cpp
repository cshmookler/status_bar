// Standard includes
#include <chrono>
#include <memory>
#include <string>
#include <thread>

// External includes
#include <gtest/gtest.h>

// Local includes
#include "../src/status.hpp"

TEST(status, time) {
    std::string time = sbar::get_time();
    EXPECT_STRNE(time.data(), sbar::error_str);
}

TEST(status, uptime) {
    sbar::System system{};
    ASSERT_FALSE(system.good());
    ASSERT_TRUE(system.init());
    ASSERT_TRUE(system.good());
    std::string uptime = sbar::get_uptime(system);
    EXPECT_STRNE(uptime.data(), sbar::error_str);
}

TEST(status, disk) {
    std::string disk_percent = sbar::get_disk_percent();
    EXPECT_STRNE(disk_percent.data(), sbar::error_str);
}

TEST(status, memory) {
    sbar::System system{};
    ASSERT_FALSE(system.good());
    ASSERT_TRUE(system.init());
    ASSERT_TRUE(system.good());
    std::string memory = sbar::get_memory_percent(system);
    EXPECT_STRNE(memory.data(), sbar::error_str);
}

TEST(status, swap) {
    sbar::System system{};
    ASSERT_FALSE(system.good());
    ASSERT_TRUE(system.init());
    ASSERT_TRUE(system.good());
    std::string swap = sbar::get_swap_percent(system);
    EXPECT_STRNE(swap.data(), sbar::error_str);
}

TEST(status, cpu) {
    sbar::Cpu cpu{};
    std::string percent = sbar::get_cpu_percent(cpu);
    EXPECT_STREQ(percent.data(), sbar::standby_str);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    percent = sbar::get_cpu_percent(cpu);
    EXPECT_STRNE(percent.data(), sbar::standby_str);
    EXPECT_STRNE(percent.data(), sbar::error_str);
}

TEST(status, cpuTemp) {
    std::string temp = sbar::get_cpu_temperature();
    EXPECT_STRNE(temp.data(), sbar::error_str);
}

TEST(status, load1) {
    sbar::System system{};
    ASSERT_FALSE(system.good());
    ASSERT_TRUE(system.init());
    ASSERT_TRUE(system.good());
    std::string load1 = sbar::get_one_minute_load_average(system);
    EXPECT_STRNE(load1.data(), sbar::error_str);
}

TEST(status, load5) {
    sbar::System system{};
    ASSERT_FALSE(system.good());
    ASSERT_TRUE(system.init());
    ASSERT_TRUE(system.good());
    std::string load5 = sbar::get_five_minute_load_average(system);
    EXPECT_STRNE(load5.data(), sbar::error_str);
}

TEST(status, load15) {
    sbar::System system{};
    ASSERT_FALSE(system.good());
    ASSERT_TRUE(system.init());
    ASSERT_TRUE(system.good());
    std::string load15 = sbar::get_fifteen_minute_load_average(system);
    EXPECT_STRNE(load15.data(), sbar::error_str);
}

TEST(status, batteryStatus) {
    sbar::Battery battery{};
    ASSERT_FALSE(battery.good());
    ASSERT_TRUE(battery.init());
    ASSERT_TRUE(battery.good());
    std::string status = sbar::get_battery_status(battery);
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, batteryDevice) {
    sbar::Battery battery{};
    ASSERT_FALSE(battery.good());
    ASSERT_TRUE(battery.init());
    ASSERT_TRUE(battery.good());
    std::string device = sbar::get_battery_device(battery);
    EXPECT_STRNE(device.data(), sbar::error_str);
}

TEST(status, battery) {
    sbar::Battery battery{};
    ASSERT_FALSE(battery.good());
    ASSERT_TRUE(battery.init());
    ASSERT_TRUE(battery.good());
    std::string percent = sbar::get_battery_percent(battery);
    EXPECT_STRNE(percent.data(), sbar::error_str);
}

TEST(status, batteryTimeRemaining) {
    sbar::Battery battery{};
    ASSERT_FALSE(battery.good());
    ASSERT_TRUE(battery.init());
    ASSERT_TRUE(battery.good());
    std::string remaining = sbar::get_battery_time_remaining(battery);
    EXPECT_STREQ(remaining.data(), sbar::standby_str);
}

TEST(status, backlight) {
    sbar::Backlight backlight{};
    ASSERT_FALSE(backlight.good());
    ASSERT_TRUE(backlight.init());
    ASSERT_TRUE(backlight.good());
    std::string percent = sbar::get_backlight_percent(backlight);
    EXPECT_STRNE(percent.data(), sbar::error_str);
}

TEST(status, networkStatus) {
    sbar::Network network{};
    ASSERT_FALSE(network.good());
    ASSERT_TRUE(network.init());
    ASSERT_TRUE(network.good());
    std::string status = sbar::get_network_status(network);
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, networkDevice) {
    sbar::Network network{};
    ASSERT_FALSE(network.good());
    ASSERT_TRUE(network.init());
    ASSERT_TRUE(network.good());
    std::string device = sbar::get_network_device(network);
    EXPECT_STRNE(device.data(), sbar::error_str);
}

TEST(status, networkSSID) {
    sbar::Network network{};
    ASSERT_FALSE(network.good());
    ASSERT_TRUE(network.init());
    ASSERT_TRUE(network.good());
    std::string ssid = sbar::get_network_ssid(network);
    EXPECT_STRNE(ssid.data(), sbar::error_str);
}

TEST(status, networkStrength) {
    sbar::Network network{};
    ASSERT_FALSE(network.good());
    ASSERT_TRUE(network.init());
    ASSERT_TRUE(network.good());
    std::string signal = sbar::get_network_signal_strength_percent(network);
    EXPECT_STRNE(signal.data(), sbar::error_str);
}

TEST(status, networkUpload) {
    sbar::Network network{};
    ASSERT_FALSE(network.good());
    ASSERT_TRUE(network.init());
    ASSERT_TRUE(network.good());
    std::string upload = sbar::get_network_upload(network);
    EXPECT_STREQ(upload.data(), sbar::standby_str);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    upload = sbar::get_network_upload(network);
    EXPECT_STRNE(upload.data(), sbar::error_str);
}

TEST(status, networkDownload) {
    sbar::Network network{};
    ASSERT_FALSE(network.good());
    ASSERT_TRUE(network.init());
    ASSERT_TRUE(network.good());
    std::string download = sbar::get_network_download(network);
    EXPECT_STREQ(download.data(), sbar::standby_str);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    download = sbar::get_network_download(network);
    EXPECT_STRNE(download.data(), sbar::error_str);
}

TEST(status, volumeStatus) {
    sbar::Sound_mixer sound_mixer{};
    ASSERT_FALSE(sound_mixer.good());
    ASSERT_TRUE(sound_mixer.init());
    ASSERT_TRUE(sound_mixer.good());
    std::string status = sbar::get_volume_status(sound_mixer);
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, volume) {
    sbar::Sound_mixer sound_mixer{};
    ASSERT_FALSE(sound_mixer.good());
    ASSERT_TRUE(sound_mixer.init());
    ASSERT_TRUE(sound_mixer.good());
    std::string volume = sbar::get_volume_perc(sound_mixer);
    EXPECT_STRNE(volume.data(), sbar::error_str);
}

TEST(status, captureStatus) {
    sbar::Sound_mixer sound_mixer{};
    ASSERT_FALSE(sound_mixer.good());
    ASSERT_TRUE(sound_mixer.init());
    ASSERT_TRUE(sound_mixer.good());
    std::string status = sbar::get_capture_status(sound_mixer);
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, capture) {
    sbar::Sound_mixer sound_mixer{};
    ASSERT_FALSE(sound_mixer.good());
    ASSERT_TRUE(sound_mixer.init());
    ASSERT_TRUE(sound_mixer.good());
    std::string capture = sbar::get_capture_perc(sound_mixer);
    EXPECT_STRNE(capture.data(), sbar::error_str);
}

TEST(status, microphone) {
    std::string microphone = sbar::get_microphone_status();
    EXPECT_STRNE(microphone.data(), sbar::error_str);
}

TEST(status, camera) {
    std::string status = sbar::get_camera_status();
    EXPECT_STRNE(status.data(), sbar::error_str);
}

TEST(status, user) {
    std::string user = sbar::get_user();
    EXPECT_STRNE(user.data(), sbar::error_str);
}

TEST(status, kernelStatus) {
    std::string kernel_status = sbar::get_outdated_kernel_indicator();
    EXPECT_STRNE(kernel_status.data(), sbar::error_str);
}
