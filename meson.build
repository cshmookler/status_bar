# Declare project information
project(
    'status_bar',
    'cpp',
    version : files('VERSION'),
    default_options : [ 'cpp_std=c++17' ],
)

# Project root and build directories
root_dir = meson.project_source_root()
build_dir = meson.project_build_root()
src_dir = root_dir / 'src'
tests_dir = root_dir / 'tests'
include_dir = root_dir / 'include'

# Insert the project version into the version header file
conf_data = configuration_data()
conf_data.set('version', meson.project_version())
configure_file(
    configuration : conf_data,
    input : include_dir / 'version.hpp.in',
    output : 'version.hpp',
)

dep_fmt = dependency(
    'fmt',
    required : true,
    method : 'auto',
)

dep_x11 = dependency(
    'x11',
    required : true,
    method : 'auto',
)

dep_alsa = dependency(
    'alsa',
    required : true,
    method : 'auto',
)

exe_status_bar = executable(
    'status_bar',
    files(
        src_dir / 'main.cpp',
        src_dir / 'version.cpp',
        src_dir / 'utils.cpp',
        src_dir / 'notify.cpp',
        src_dir / 'watcher.cpp',
        src_dir / 'status.cpp',
        src_dir / 'root_window.cpp',
        src_dir / 'components' / 'time.cpp',
        src_dir / 'components' / 'memory.cpp',
        src_dir / 'components' / 'cpu.cpp',
        src_dir / 'components' / 'load_average.cpp',
        src_dir / 'components' / 'battery.cpp',
        src_dir / 'components' / 'backlight.cpp',
        src_dir / 'components' / 'network.cpp',
        src_dir / 'components' / 'sound.cpp',
        src_dir / 'components' / 'microphone.cpp',
        src_dir / 'components' / 'camera.cpp',
        src_dir / 'components' / 'user.cpp',
        src_dir / 'components' / 'kernel_status.cpp',
    ),
    dependencies : [ dep_fmt, dep_x11, dep_alsa ],
    install : true,
)

lib_version = shared_library(
    'status_bar_version',
    files(
        src_dir / 'version.cpp',
    ),
    version : meson.project_version(),
    install : true,
)
lib_notify = shared_library(
    'status_bar_notify',
    files(
        src_dir / 'notify.cpp',
    ),
    version : meson.project_version(),
    install : true,
)
lib_watcher = shared_library(
    'status_bar_watcher',
    files(
        src_dir / 'watcher.cpp',
    ),
    version : meson.project_version(),
    install : true,
)

install_headers(
    files(
        build_dir / 'version.hpp',
        include_dir / 'notify.hpp',
        include_dir / 'watcher.hpp',
    ),
    subdir : 'status_bar',
)

dep_gtest_main = dependency(
    'gtest_main',
    required : false,
    method : 'auto',
)

if dep_gtest_main.found()
    test_version = executable(
        'version',
        files(
            tests_dir / 'version_test.cpp',
            src_dir / 'version.cpp',
        ),
        dependencies : dep_gtest_main,
    )
    test('version', test_version)

    test_status = executable(
        'status',
        files(
            tests_dir / 'status_test.cpp',
            src_dir / 'utils.cpp',
            src_dir / 'status.cpp',
            src_dir / 'components' / 'time.cpp',
            src_dir / 'components' / 'memory.cpp',
            src_dir / 'components' / 'cpu.cpp',
            src_dir / 'components' / 'load_average.cpp',
            src_dir / 'components' / 'battery.cpp',
            src_dir / 'components' / 'backlight.cpp',
            src_dir / 'components' / 'network.cpp',
            src_dir / 'components' / 'sound.cpp',
            src_dir / 'components' / 'microphone.cpp',
            src_dir / 'components' / 'camera.cpp',
            src_dir / 'components' / 'user.cpp',
            src_dir / 'components' / 'kernel_status.cpp',
        ),
        dependencies : [dep_gtest_main, dep_alsa],
    )
    test('status', test_status)

    test_notify = executable(
        'notify',
        files(
            tests_dir / 'notify_test.cpp',
        ),
        dependencies : [dep_gtest_main],
    )
    test('notify', test_notify)
else
    warning('Skipping tests due to missing dependencies')
endif
