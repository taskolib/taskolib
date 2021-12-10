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
    avto::LuaState lua_state;

    luaL_openlibs(lua_state.get()); // Load Lua libraries

    // Load the script we are going to run from the string
    lua_state.load_string(the_script);

    /*
     * Ok, now here we go: We pass data to the lua script on the stack.
     * That is, we first have to prepare Lua's virtual stack the way we
     * want the script to receive it, then ask Lua to run it.
     */

    // Push an empty table onto the stack
    lua_state.create_table();

    /*
     * To put values into the table, we first push the index, then the
     * value, and then call lua_rawset() with the index of the table in the
     * stack. Let's see why it's -3: In Lua, the value -1 always refers to
     * the top of the stack. When you create the table with lua_newtable(),
     * the table gets pushed into the top of the stack. When you push the
     * index and then the cell value, the stack looks like:
     *
     * <- [stack bottom] -- table, index, value [top]
     *
     * So the -1 will refer to the cell value, thus -3 is used to refer to
     * the table itself. Note that lua_rawset() pops the two last elements
     * of the stack, so that after it has been called, the table is at the
     * top of the stack.
     */
    for (int i = 1; i <= 5; ++i)
    {
        lua_state.push_number(i);   // Push the table index
        lua_state.push_number(i*2); // Push the cell value
        lua_state.set_table(-3);    // Store the pair in the table
    }

    // By what name is the script going to reference our table?
    lua_state.set_global("foo");

    // Install a hook that is called after every (1) LUA instruction
    lua_sethook(lua_state.get(), check_script_timeout, LUA_MASKCOUNT, 1);

    t0 = gul14::tic();

    // Ask Lua to run our little script
    int err = lua_pcall(lua_state.get(), 0, LUA_MULTRET, 0);
    if (err)
    {
        std::cerr << "Error while executing script: " << lua_tostring(lua_state.get(), -1) << "\n";
        exit(1);
    }

    std::cout << "Script returned: " << lua_state.pop_number() << "\n";

    return 0;
}
