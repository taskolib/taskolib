project(
    'taskomat',
    ['cpp'],
    version : run_command([
            find_program('cat'),
            meson.current_source_dir() / 'LIBNO'
        ],
        check : true,
    ).stdout().split('=')[1].strip(),
    default_options : [
        'cpp_std=c++17',
        'warning_level=3',
        'buildtype=release',
        'prefix=/usr',
        'libdir=lib',
    ],
    meson_version : '>=0.49'
)

pkg_config_name = 'libtaskomat'

# Configuration for included Sol3 library
# SOL_USING_CXX_LUA tells Sol3 that we are using a custom LUA build that was compiled
# with C++ and therefore properly handles exceptions.
# SOL_SAFE_NUMERICS tells Sol3 to make several checks so that we can actually
# differentiate integers from floating-point numbers.
#
# Marcus (27.01.2022): Change SOL_ALL_SAFETIES_ON from 0 (OFF) to 1 (ON). What is the meaning of it?
add_project_arguments(
    '-DSOL_USING_CXX_LUA=1', '-DSOL_ALL_SAFETIES_ON=1', '-DSOL_PRINT_ERRORS=0',
    '-DSOL_SAFE_NUMERICS=1',
    language : 'cpp')


## File lists

# Headers to be installed under ${prefix}/include/
install_headers(files([
    'include/taskomat/Context.h',
    'include/taskomat/Error.h',
    'include/taskomat/execute_step.h',
    'include/taskomat/execute_sequence.h',
    'include/taskomat/Sequence.h',
    'include/taskomat/Step.h',
    'include/taskomat/taskomat.h',
    'include/taskomat/VariableName.h',
    ]),
    subdir: 'taskomat')

# Sources
sources = files([
    'src/execute_step.cc',
    'src/execute_sequence.cc',
    'src/Sequence.cc',
    'src/Step.cc',
    'src/VariableName.cc',
])

# Test sources
test_src = files([
    'tests/test_Context.cc',
    'tests/test_Error.cc',
    'tests/test_execute_step.cc',
    'tests/test_execute_sequence.cc',
    'tests/test_main.cc',
    'tests/test_Sequence.cc',
    'tests/test_Step.cc',
    'tests/test_VariableName.cc',
])


## Determine the version number

# By default, the version number is simply the project version. If an alternative version
# is specified by a Meson command line switch like "-Dversion=21.7.2-precise5", it is
# used instead. The version part has at least 3 unsigned numerals separated by dots X.Y.Z
# with all numerals < 256.
# Any additional patchlevel string, consisting of alnum characters and appended with
# a dot separator, is used unchanged for the dynamic library version (except on Darwin,
# where it is ignored): X.Y.Z.p
# Note that it can be problematic to use '-' or '_' in the p part.

libno_raw = get_option('version')
if libno_raw == ''
    libno_raw = meson.project_version()
endif
libno_parts = libno_raw.split('.')
if libno_parts.length() < 3
    error('Version number "' + libno_raw + '" does not contain three fields X.Y.Z[.p]')
endif
libno = '.'.join([ libno_parts[0], libno_parts[1], libno_parts[2].split('-')[0] ])
libno_full = '.'.join(libno_parts)


# Generate the static LUA library
# It can be used via lua_dep
subdir('src/lua')


## Build library
inc = [ include_directories('include', 'src') ]

deps = [
    dependency('libgul14'),
]

lib = both_libraries(meson.project_name(),
    sources,
    dependencies : deps + [ lua_dep ],
    soversion : libno_full,
    darwin_versions : libno,
    include_directories : inc,
    build_rpath : meson.current_build_dir(),
    install_rpath : get_option('prefix') / get_option('libdir'),
    #gnu_symbol_visibility : 'hidden',
    install : true)


## pkg-config

pkg = import('pkgconfig')
pkg.generate(lib,
    name : pkg_config_name,
    description : 'Taskomat Library',
    version : libno,
    filebase : pkg_config_name,
    libraries : [ '-Wl,-rpath,${libdir}' ],
    requires : [ 'libgul14' ],
)

taskomat_dep = declare_dependency(
    include_directories : inc,
    link_with : lib,
    dependencies : deps,
)

## Tests

# The tests are executed in the build dir to avoid pollution of the git repository with
# the test executable's output files
test('all',
    executable(meson.project_name() + '_test',
        test_src,
        dependencies : [taskomat_dep, lua_dep],
    ),
    workdir : meson.current_build_dir(),
    timeout : 10,
)


## Examples

executable('execute_step',
    files(['examples/execute_step.cc']),
    dependencies : taskomat_dep
)

## Include experimental sources for lua/sol. All of the buiild executable will start with
## 'experiment_...' under folder 'playground'. To disable it you only need to comment it
## out.
subdir('playground')

## Build message

message('Building library version @0@'.format(libno))
if libno != libno_full
    message('(!)          .so version @0@'.format(libno_full))
endif
if meson.project_version().version_compare('!= ' + libno)
    message('(!)      project version @0@'.format(meson.project_version()))
endif
