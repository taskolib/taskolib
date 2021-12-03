#include <iostream>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main()
{
    /*
     * All Lua contexts are held in this structure. We work with it almost
     * all the time.
     */
    lua_State* lua_state = luaL_newstate();

    luaL_openlibs(lua_state); /* Load Lua libraries */

    /* Load the file containing the script we are going to run */
    int status = luaL_loadfile(lua_state, "script.lua");
    if (status)
    {
        // If something went wrong, error message is at the top of the stack
        std::cerr << "Couldn't load file: " << lua_tostring(lua_state, -1) << "\n";
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
    for (int i = 1; i <= 5; i++) {
        lua_pushnumber(lua_state, i);   // Push the table index
        lua_pushnumber(lua_state, i*2); // Push the cell value
        lua_rawset(lua_state, -3);      // Stores the pair in the table
    }

    // By what name is the script going to reference our table?
    lua_setglobal(lua_state, "foo");

    // Ask Lua to run our little script
    int result = lua_pcall(lua_state, 0, LUA_MULTRET, 0);
    if (result)
    {
        std::cerr << "Failed to run script: " << lua_tostring(lua_state, -1) << "\n";
        exit(1);
    }

    // Get the returned value at the top of the stack (index -1)
    double sum = lua_tonumber(lua_state, -1);

    std::cout << "Script returned: " << sum << "\n";

    lua_pop(lua_state, 1);  // Take the returned value out of the stack
    lua_close(lua_state);   // Cya, Lua

    return 0;
}
