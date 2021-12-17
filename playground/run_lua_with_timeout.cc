#include <gul14/gul.h>
#include <iostream>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../include/avtomat/LuaState.h"

const std::string the_script =
R"(
-- An infinite loop
while true do
    io.write("Infinite loop!\n");
end
)";

std::chrono::steady_clock::time_point t0;


void check_script_timeout(lua_State* lua_state, lua_Debug*) noexcept
{
    constexpr int timeout_s = 3;

    if (gul14::toc(t0) > timeout_s)
    {
        const std::string msg = gul14::cat("Timeout: Script took more than ", timeout_s,
                                           " seconds");
        luaL_error(lua_state, msg.c_str());
    }
}

int main()
{
    avto::LuaState lua_state(avto::LuaLibraries::all);

    // Load the script we are going to run from the string and put the resulting chunk
    // onto the LUA stack
    lua_state.load_string(the_script);

    // Create a table on the stack and fill it with some values
    lua_state.create_table();

    for (int i = 1; i <= 5; ++i)
        lua_state.assign_field(i, i * 2);

    // Assign a global name to the table (and pop it from the stack)
    lua_state.set_global("foo");

    // Install a hook that is called after every (1) LUA instruction
    lua_sethook(lua_state.get(), check_script_timeout, LUA_MASKCOUNT, 1);

    t0 = gul14::tic();

    // Ask Lua to run our little script
    int num_results = lua_state.call_function();

    std::cout << "Script returned:\n";
    for (int i = 0; i != num_results; ++i)
        std::cout << "    " << lua_state.pop_number() << "\n";

    return 0;
}
