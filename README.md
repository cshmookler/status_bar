# **status_bar**

Status bar for [dwm](https://dwm.suckless.org) on [MOOS](https://github.com/cshmookler/moos). Customizable at runtime and updates instantly.

## Build from Source

### 1.&nbsp; Install a C++ compiler, Meson, GoogleTest (optional), argparse, X11 client-side libraries, [cpp_result](https://github.com/cshmookler/cpp_result), [system_state](https://github.com/cshmookler/system_state), and [inotify_ipc](https://github.com/cshmookler/inotify_ipc).

#### Linux (MOOS):

```bash
sudo pacman -S base-devel meson gtest argparse libx11 moos-cpp-result moos-system-state moos-inotify-ipc
```

### 2.&nbsp; Clone this project.

This project can be downloaded online [here](https://github.com/cshmookler/status_bar).

Alternatively, if you have [Git](https://git-scm.com/downloads/) installed, open a shell and enter the commands below.  This project will be downloaded to the current working directory.

```bash
git clone https://github.com/cshmookler/status_bar.git
cd status_bar
```

### 3.&nbsp; Build this project from source.

```bash
meson setup build
cd build
ninja
```

### 4.&nbsp; (Optional) Install this project globally.

```bash
meson install
```

## **TODO**

- [X] date and time
- [X] uptime
- [ ] disks
- [ ] disk space
- [ ] disk partitions
- [X] swap
- [X] memory
- [X] cpu usage
- [X] highest measured temperature
- [X] lowest measured temperature
- [X] load averages (1 min, 5 min, 15 min)
- [X] battery status
- [X] battery name
- [X] battery charge
- [X] battery capacity
- [X] battery current
- [X] battery power
- [X] battery time remaining
- [X] backlight percentage
- [X] network status
- [X] network name
- [X] network packets up
- [X] network packets down
- [X] network bytes up
- [X] network bytes down
- [ ] network SSID
- [ ] network signal strength percentage
- [X] volume mute
- [X] volume percentage
- [X] capture mute
- [X] capture percentage
- [ ] microphone status
- [ ] camera status
- [X] user name
- [X] outdated kernel indicator
