# **status_bar**

Status bar for [dwm](https://dwm.suckless.org). Customizable at runtime and updates instantly.

## Build from Source

### 1.&nbsp; Install a C++ compiler, Meson, GoogleTest (optional), [{fmt}](https://fmt.dev), argparse, X11 client-side libraries, and ALSA libraries.

#### Linux (Arch):

```bash
sudo pacman -S base-devel meson gtest fmt argparse libx11 alsa-lib
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

## **TODO**

- [X] date and time
- [X] uptime
- [X] disk space
- [X] swap
- [X] memory
- [X] cpu usage
- [X] cpu temperature
- [X] load averages (1 min, 5 min, 15 min)
- [X] battery status
- [X] battery name
- [X] battery percentage
- [X] battery time remaining
- [X] backlight percentage
- [X] network status
- [X] network name
- [X] network SSID
- [X] network signal strength percentage
- [X] volume mute
- [X] volume percentage
- [X] capture mute
- [X] capture percentage
- [X] microphone status
- [ ] camera status
- [X] user name
- [X] outdated kernel indicator
