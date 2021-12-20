project(
  'libavtomat',
  ['cpp'],
version : '21.12.1',
default_options : [
    'cpp_std=c++17',
    'warning_level=3',
    'buildtype=release',
    'prefix=/export/doocs',
    'libdir=lib',
    'includedir=lib/include'
  ],
meson_version : '>=0.49'
)

package_name = 'doocs-libavtomat'


## File lists

# Headers to be installed under ${prefix}/include/
install_headers(files([
    'include/avtomat/avtomat.h',
    'include/avtomat/Context.h',
    'include/avtomat/Error.h',
    'include/avtomat/execute_step.h',
    'include/avtomat/LuaState.h',
    'include/avtomat/Step.h',
    ]),
    subdir: 'avtomat')

# Sources
sources = files([
    'src/execute_step.cc',
    'src/LuaState.cc',
    'src/Step.cc',
])

# Test sources
test_src = files([
    'tests/test_Context.cc',
    'tests/test_Error.cc',
    'tests/test_execute_step.cc',
    'tests/test_LuaState.cc',
    'tests/test_main.cc',
    'tests/test_Step.cc',
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


## Build library
inc = [ include_directories('include', 'src') ]

deps = [
    dependency('libgul14'),
    dependency('lua5.3'),
]

lib = both_libraries(meson.project_name(),
    sources,
    dependencies : deps,
    soversion : libno_full,
    darwin_versions : libno,
    include_directories : inc,
    build_rpath : meson.current_build_dir(),
    install_rpath : join_paths(get_option('prefix'),get_option('libdir')),
    #gnu_symbol_visibility : 'hidden',
    install : true)


## pkg-config

pkg = import('pkgconfig')
pkg.generate(lib,
    name : package_name,
    description : 'Avtomat Library',
    version : libno,
    filebase : package_name,
    libraries : [ '-Wl,-rpath,${libdir}' ],
    requires : [ 'libgul14' ],
)


## Tests

dep = declare_dependency(
    include_directories : inc,
    link_with : lib,
    dependencies : deps,
)

# The tests are executed in the build dir to avoid pollution of the git repository with
# the test executable's output files
test('all',
    executable(meson.project_name() + '_test',
        test_src,
        dependencies : dep,
        include_directories : inc,
    ),
    workdir : meson.current_build_dir(),
    timeout : 10,
)


## "Playground" executables

executable('run_lua',
    files(['playground/run_lua.cc']),
    dependencies : dep
)

executable('run_lua_with_timeout',
    files([
        'playground/run_lua_with_timeout.cc',
        ]),
    dependencies : dep
)


## Build message

message('Building library version @0@'.format(libno))
if libno != libno_full
    message('(!)          .so version @0@'.format(libno_full))
endif
if meson.project_version().version_compare('!= ' + libno)
    message('(!)      project version @0@'.format(meson.project_version()))
endif
