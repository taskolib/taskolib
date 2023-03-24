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
lua_version = '5.4.3'



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


## Build library
subdir('include')
subdir('src')
subdir('src/lua')

inc = [ include_directories('include', 'include/taskolib/sol', 'src') ]

gul_dep = dependency('libgul14', version : '> 2.6', fallback : ['libgul14', 'libgul_dep'])

deps = [
    dependency('threads'),
    gul_dep.partial_dependency(compile_args : true, includes : true),
    lua_dep,
    dependency('libgit2')
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
