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

dep_gio = dependency(
    'gio-2.0',
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
        root_dir / 'src' / 'main.cpp',
        root_dir / 'src' / 'version.cpp',
        root_dir / 'src' / 'status.cpp',
    ),
    dependencies : [ dep_x11, dep_gio, dep_alsa ],
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
            'src' / 'version.test.cpp',
            'src' / 'version.cpp',
        ),
        dependencies : dep_gtest_main,
    )
    test('version', test_version)
else
    warning('Skipping tests due to missing dependencies')
endif
