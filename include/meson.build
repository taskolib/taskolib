# Headers to be installed under ${prefix}/include/
public_headers = [
   'taskolib/CommChannel.h',
   'taskolib/Context.h',
   'taskolib/default_message_callback.h',
   'taskolib/deserialize_sequence.h',
   'taskolib/exceptions.h',
   'taskolib/execute_lua_script.h',
   'taskolib/Executor.h',
   'taskolib/format.h',
   'taskolib/GitObjectWrapper.h',
   'taskolib/GitRepository.h',
   'taskolib/hash_string.h',
   'taskolib/LockedQueue.h',
   'taskolib/Message.h',
   'taskolib/Sequence.h',
   'taskolib/SequenceManager.h',
   'taskolib/serialize_sequence.h',
   'taskolib/Step.h',
   'taskolib/StepIndex.h',
   'taskolib/taskolib.h',
   'taskolib/time_types.h',
   'taskolib/Timeout.h',
   'taskolib/VariableName.h'
]
install_headers(files(public_headers),
    subdir: 'taskolib',
)

install_subdir('taskolib/sol/sol',
    install_dir: get_option('prefix') / get_option('includedir') / 'taskolib' / 'sol' + sol_version,
)

lua_headers = files(
    'taskolib/lua/lauxlib.h',
    'taskolib/lua/lua.h',
    'taskolib/lua/luaconf.h',
    'taskolib/lua/lualib.h',
)
install_headers(lua_headers, subdir: 'taskolib' / 'lua' + lua_version)

lua_inc = include_directories('taskolib/lua') # used by src/lua
