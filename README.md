[![Python 3.7+](https://img.shields.io/badge/python-%3E%3D%20v3.7-blue)](https://www.python.org/downloads/release/python-370/)
[![](https://badgen.net/github/last-commit/cshmookler/status_bar)](https://github.com/cshmookler/status_bar/commits/master)
[![](https://badgen.net/github/license/cshmookler/status_bar)](https://github.com/cshmookler/status_bar/blob/master/LICENSE)


# **status_bar**

Status bar for dwm. Customizable at runtime and updates instantly.

## **Build and install this project with Conan (for Unix-like systems)**

**1.** Install a C++ compiler (Example: clang), Git, and Python >=3.7 (Example: apt).

```bash
sudo apt install -y clang git python3
```

**2.** Run the build script.

```bash
python3 build.py
```

<details>
<summary> <strong>Click here if you get an error while building</strong> </summary>

#### Failed to build dependency from source

```
CMake Error at /usr/local/share/cmake-3.26/Modules/CmakeTestCXXCompiler.cmake:60 (message):
  The C++ compiler

    "/usr/bin/c++"

  is not able to compile a simple test program.
```

<details>
<summary> <strong>Click here if this is your error</strong> </summary>

A dependency likely passed invalid compiler flags. Try using a different compiler.

**1.** Clear the Conan cache.

```bash
python3 clear_cache.py
```

**2.** Remove build files.

```bash
python3 clean.py
```

**3.** Set a different compiler for CMake to use.

- For Clang:

```bash
export CC=clang
export CXX=clang++
```

- For GCC:

```bash
export CC=gcc
export CXX=g++
```

**4.** Rerun the build script.

```bash
python3 build.py
```

</details>

#### Conan related error

See the official [Conan FAQ](https://docs.conan.io/2/knowledge/faq.html) for help with common errors.

</details>

## **TODO**

- [X] date and time
- [ ] disk space
- [ ] memory
- [ ] cpu usage
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
