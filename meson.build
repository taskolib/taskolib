project(
    'taskolib',
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
        'b_lundef=false', # use but do not link GUL
    ],
    meson_version : '>=0.49'
)

pkg_config_name = 'taskolib'

# Keep this version number up to date...
sol_version = '3.3.0'


## File lists

# Headers to be installed under ${prefix}/include/
public_headers = [
   'include/taskolib/CommChannel.h',
   'include/taskolib/console.h',
   'include/taskolib/Context.h',
   'include/taskolib/deserialize_sequence.h',
   'include/taskolib/exceptions.h',
   'include/taskolib/Executor.h',
   'include/taskolib/format.h',
   'include/taskolib/hash_string.h',
   'include/taskolib/LockedQueue.h',
   'include/taskolib/Message.h',
   'include/taskolib/Sequence.h',
   'include/taskolib/SequenceManager.h',
   'include/taskolib/serialize_sequence.h',
   'include/taskolib/Step.h',
   'include/taskolib/StepIndex.h',
   'include/taskolib/taskolib.h',
   'include/taskolib/time_types.h',
   'include/taskolib/VariableName.h',
]
install_headers(files(public_headers),
    subdir: 'taskolib',
)

install_subdir('include/taskolib/sol/sol',
    install_dir: get_option('prefix') / get_option('includedir') / 'taskolib' / 'sol' + sol_version,
)

# Sources
sources = files(
    'src/console.cc',
    'src/deserialize_sequence.cc',
    'src/Executor.cc',
    'src/internals.cc',
    'src/lua_details.cc',
    'src/Sequence.cc',
    'src/SequenceManager.cc',
    'src/serialize_sequence.cc',
    'src/Step.cc',
    'src/time_types.cc',
    'src/VariableName.cc',
)


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

## Add slightly modified dependency for std::filesystem during linking (on some Linux OS,
## for example Debian 10, "Buster", we do not have an automated linker option although we
## still building on C++17 standard - amazing)
## See StackOverflow:
## https://stackoverflow.com/questions/52516165/cannot-use-the-c-stdfilesystem-library-with-meson-build
add_project_link_arguments(['-lstdc++fs'], language : 'cpp')

# Generate the static LUA library
# It can be used via lua_dep
lua_inc = include_directories('include/taskolib/lua') # used by src/lua
subdir('src/lua')

install_headers(files(
        'include/taskolib/lua/lauxlib.h',
        'include/taskolib/lua/lua.h',
        'include/taskolib/lua/luaconf.h',
        'include/taskolib/lua/lualib.h',
    ),
    subdir: 'taskolib' / 'lua' + lua_version)


## Build library
inc = [ include_directories('include', 'include/taskolib/sol', 'src') ]

gul_dep = dependency('libgul14')

deps = [
    dependency('threads'),
    gul_dep.partial_dependency(compile_args : true, includes : true),
    lua_dep,
]

lib = both_libraries(meson.project_name(),
    sources,
    dependencies : deps,
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
    description : 'Taskolib',
    version : libno,
    filebase : pkg_config_name,
    libraries : [ '-Wl,-rpath,${libdir}' ],
    requires : [ 'libgul14' ],
    subdirs : [ '', 'taskolib' / 'sol' + sol_version, 'taskolib' / 'lua' + lua_version, ],
)

taskolib_dep = declare_dependency(
    include_directories : inc,
    link_with : lib,
    dependencies : [ deps, gul_dep, ],
)

subdir('tests')

## Examples

executable('execute_step',
    files(['examples/execute_step.cc']),
    dependencies : taskolib_dep
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
