project(
    'Containers',
    'cpp',
    version: '0.1.0',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
    ],
    meson_version: '>=0.63.0',
)

# Header-only library: include dir + dependency object
inc = include_directories('include')

containers_dep = declare_dependency(include_directories: inc)
# Allow use as a subproject / wrap: `dependency('containers')` falls back to this
meson.override_dependency('containers', containers_dep)

# Install headers (contents of include/ go straight into the include dir)
install_subdir('include/', install_dir: get_option('includedir'), strip_directory: true)

# pkg-config file (replaces ContainersConfig.cmake / ContainersTargets.cmake)
pkg = import('pkgconfig')
pkg.generate(
    name: 'Containers',
    filebase: 'Containers',
    description: 'Header-only containers library',
    version: meson.project_version()
)

# Tests
containers_tests = executable(
    'ContainersTests',
    'tests/main.cpp',
    'tests/BuddyAllocatorTests.cpp',
    dependencies: containers_dep,
    # Meson already sets cpp_std=c++20 via default_options; this enforces it
    # for anyone consuming the project directly too.
    override_options: ['cpp_std=c++20'],
)

test('ContainersTests', containers_tests)