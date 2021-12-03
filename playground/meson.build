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
]

executable('run_lua',
    files(['run_lua.cc']),
    dependencies : deps
)
