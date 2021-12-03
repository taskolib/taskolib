project(
  'playground',
  ['cpp'],
version : '21.7.1',
default_options : [
    'cpp_std=c++17',
    'warning_level=3',
    'buildtype=debug',
  ],
meson_version : '>=0.49'
)

deps = [
    dependency('lua5.3'),
    dependency('libgul14'),
]

executable('run_lua',
    files(['run_lua.cc']),
    dependencies : deps
)

executable('run_lua_with_timeout',
    files(['run_lua_with_timeout.cc']),
    dependencies : deps
)
