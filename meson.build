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

# Insert the project version into the version header file
conf_data = configuration_data()
conf_data.set('version', meson.project_version())
configure_file(
    configuration : conf_data,
    input : 'src' / 'version.hpp.in',
    output : 'version.hpp',
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
        src_dir / 'status.cpp',
        src_dir / 'components' / 'time.cpp',
        src_dir / 'components' / 'memory.cpp',
        src_dir / 'components' / 'load_average.cpp',
        src_dir / 'components' / 'battery.cpp',
        src_dir / 'components' / 'network.cpp',
        src_dir / 'components' / 'sound.cpp',
    ),
    dependencies : [ dep_x11, dep_alsa ],
    install : true,
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
            src_dir / 'version.test.cpp',
            src_dir / 'version.cpp',
        ),
        dependencies : dep_gtest_main,
    )
    test('version', test_version)

    test_status = executable(
        'status',
        files(
            src_dir / 'status.test.cpp',
            src_dir / 'utils.cpp',
            src_dir / 'status.cpp',
            src_dir / 'components' / 'time.cpp',
            src_dir / 'components' / 'memory.cpp',
            src_dir / 'components' / 'load_average.cpp',
            src_dir / 'components' / 'battery.cpp',
            src_dir / 'components' / 'network.cpp',
            src_dir / 'components' / 'sound.cpp',
        ),
        dependencies : [dep_gtest_main, dep_alsa],
    )
    test('status', test_status)
else
    warning('Skipping tests due to missing dependencies')
endif
