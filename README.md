# **status_bar**

Status bar for [dwm](https://dwm.suckless.org). Customizable at runtime and updates instantly.

## Build from Source

### 1.&nbsp; Install Git, Python, and a C++ compiler

#### Linux (Ubuntu):

```bash
sudo apt install git python3 build-essential
```

#### Linux (Arch):

```bash
sudo pacman -S git python base-devel
```

### 2.&nbsp; Clone this template

```
git clone https://github.com/cshmookler/status_bar.git
cd status_bar
```

### 3.&nbsp; Build from source

```
python build.py
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
- [ ] battery percentage
- [ ] battery time remaining
- [ ] backlight percentage
- [ ] network SSID
- [ ] wifi percentage
- [ ] bluetooth device
- [ ] volume mute
- [ ] volume percentage
- [ ] microphone status
- [ ] camera status
