# Source files for our Lua library compiled as C++
lua_sources = files(
    'lapi.cc',
    'lauxlib.cc',
    'lbaselib.cc',
    'lcode.cc',
    'lcorolib.cc',
    'lctype.cc',
    'ldblib.cc',
    'ldebug.cc',
    'ldo.cc',
    'ldump.cc',
    'lfunc.cc',
    'lgc.cc',
    'linit.cc',
    'liolib.cc',
    'llex.cc',
    'lmathlib.cc',
    'lmem.cc',
    'loadlib.cc',
    'lobject.cc',
    'lopcodes.cc',
    'loslib.cc',
    'lparser.cc',
    'lstate.cc',
    'lstring.cc',
    'lstrlib.cc',
    'ltable.cc',
    'ltablib.cc',
    'ltm.cc',
    'lundump.cc',
    'lutf8lib.cc',
    'lvm.cc',
    'lzio.cc',
)

# Build Lua library
lua_lib = static_library(
    'lua',
    lua_sources,
    override_options : [
        'warning_level=2',
        'buildtype=release',
    ],
    include_directories : lua_inc,
)

# The generated static Lua library
# can be used via lua_dep
lua_dep = declare_dependency(
    link_with : lua_lib,
    include_directories : lua_inc,
)
