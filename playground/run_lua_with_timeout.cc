#include <gul14/gul.h>
#include <iostream>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../include/avtomat/LuaState.h"

const std::string the_script =
R"(
-- An infinite loop
io.write("The table the script received has:\n");
x = 0
for i = 1, #foo do
  print(i, foo[i])
  x = x + foo[i]
end
while true do
    io.write("Infinite loop!\n");
end
io.write("Returning data back to C\n");
return x
)";

std::chrono::steady_clock::time_point t0;


void check_script_timeout(lua_State* lua_state, lua_Debug*) noexcept
{
    constexpr int timeout_s = 5;

    if (gul14::toc(t0) > timeout_s)
    {
        const std::string msg = gul14::cat("Timeout: Script took more than ", timeout_s,
                                           " seconds");
        luaL_error(lua_state, msg.c_str());
    }
}

int main()
{
    avto::LuaState the_lua_state;
    auto lua_state = the_lua_state.get();

    luaL_openlibs(lua_state); // Load Lua libraries

    // Load the script we are going to run from the string
    int status = luaL_loadstring(lua_state, the_script.c_str());
    if (status)
    {
        // If something went wrong, error message is at the top of the stack
        std::cerr << "Couldn't load script: " << lua_tostring(lua_state, -1) << "\n";
        exit(1);
    }

    /*
     * Ok, now here we go: We pass data to the lua script on the stack.
     * That is, we first have to prepare Lua's virtual stack the way we
     * want the script to receive it, then ask Lua to run it.
     */
    lua_newtable(lua_state);    // We will pass a table

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
    for (int i = 1; i <= 5; i++)
    {
        lua_pushnumber(lua_state, i);   // Push the table index
        lua_pushnumber(lua_state, i*2); // Push the cell value
        lua_rawset(lua_state, -3);      // Stores the pair in the table
    }

    // By what name is the script going to reference our table?
    lua_setglobal(lua_state, "foo");

    // Install a hook that is called after every (1) LUA instruction
    lua_sethook(lua_state, check_script_timeout, LUA_MASKCOUNT, 1);

    t0 = gul14::tic();

    // Ask Lua to run our little script
    int err = lua_pcall(lua_state, 0, LUA_MULTRET, 0);
    if (err)
    {
        std::cerr << "Error while executing script: " << lua_tostring(lua_state, -1) << "\n";
        exit(1);
    }

    // Get the returned value at the top of the stack (index -1)
    double sum = lua_tonumber(lua_state, -1);

    std::cout << "Script returned: " << sum << "\n";

    lua_pop(lua_state, 1);  // Take the returned value out of the stack

    return 0;
}
