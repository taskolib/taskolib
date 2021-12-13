#include <gul14/gul.h>
#include <iostream>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../include/avtomat/LuaState.h"

const std::string the_script =
R"(
-- script.lua
-- Receives a table, returns the sum of its components.
io.write("The table the script received has:\n");
x = 0
for i = 1, #foo do
  print(i, foo[i])
  x = x + foo[i]
end
io.write("Returning data back to C\n");
return x
)";

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

    // Push an empty table onto the stack, then fill it with some key-value pairs
    lua_state.create_table();
    for (int i = 1; i <= 5; ++i)
        lua_state.assign_field(i, i * 2);

    // By what name is the script going to reference our table?
    lua_state.set_global("foo");

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
