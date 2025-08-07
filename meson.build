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
    input : include_dir / 'version.h.in',
    output : 'version.h',
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
cpp = meson.get_compiler('cpp')
lib_system_state = cpp.find_library(
    'system_state',
    required : true,
)
lib_inotify_ipc = cpp.find_library(
    'inotify_ipc',
    required : true,
)

exe_status_bar = executable(
    'status_bar',
    files(
        src_dir / 'main.cpp',
        src_dir / 'version.cpp',
        src_dir / 'root_window.cpp',
        src_dir / 'notify.cpp',
    ),
    dependencies : [ dep_x11, dep_alsa, lib_system_state, lib_inotify_ipc ],
    install : true,
)

lib_status_bar_notify_headers = files(
    build_dir / 'version.h',
    include_dir / 'notify.h',
)
lib_status_bar_notify = library(
    'status_bar_notify',
    files(
        src_dir / 'version.cpp',
        src_dir / 'notify.cpp',
    ),
    version : meson.project_version(),
    dependencies : [ lib_inotify_ipc ],
    install : true,
)
install_headers(lib_status_bar_notify_headers, subdir : 'status_bar')

dep_gtest_main = dependency(
    'gtest_main',
    required : false,
    method : 'auto',
)

if dep_gtest_main.found()
    test_version = executable(
        'version',
        files(
            tests_dir / 'version.test.cpp',
            src_dir / 'version.cpp',
        ),
        dependencies : dep_gtest_main,
    )
    test('version', test_version)
else
    warning('Skipping tests due to missing dependencies')
endif
