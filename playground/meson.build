project(
  'run_lua',
  ['cpp'],
version : '21.7.1',
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
#install_headers(files([
#    'include/avtomat/avtomat.h',
#    'include/avtomat/Step.h',
#    ]),
#    subdir: 'avtomat')

# Sources
sources = files([
    'run_lua.cc',
])

# Test sources
#test_src = files([
#    'tests/test_main.cc',
#    'tests/test_Step.cc',
#])


deps = [
    dependency('lua5.3'),
]

## Tests

dep = declare_dependency(
    dependencies : deps,
)

# The tests are executed in the build dir to avoid pollution of the git repository with
# the test executable's output files
executable(meson.project_name(),
    sources,
    dependencies : dep,
)
