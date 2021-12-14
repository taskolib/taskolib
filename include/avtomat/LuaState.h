/**
 * \file   LuaState.h
 * \author Lars Froehlich
 * \date   Created on December 3, 2021
 * \brief  Declaration of the LuaState and LuaType classes.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the license, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef AVTOMAT_LUASTATE_H_
#define AVTOMAT_LUASTATE_H_

#include <string>
#include <type_traits>
#include "avtomat/Error.h"

struct lua_State; // Forward declaration of struct lua_State from the LUA headers


namespace avto {

/**
 * An enum for the types of values that can exist on the LUA stack.
 *
 * The values are identical to the LUA constants LUA_TNONE, LUA_TNIL, LUA_TBOOLEAN etc.,
 * but are repeated here to isolate the user from the LUA C headers. Unit tests ensure
 * matching numeric constants.
 */
enum class LuaType : int
{
    none = -1, nil = 0, boolean = 1, light_user_data = 2, number = 3,
    string = 4, table = 5, function = 6, user_data = 7, thread = 8
};

/**
 * This class encapsulates a state of the LUA virtual machine.
 * It offers only a small subset of the functionalities of LUA.
 *
 * \note
 * LuaState is a move-only class. Moved-from objects represent a "closed" LUA state. They
 * can be deleted safely, but other operations on them may invoke undefined behavior.
 */
class LuaState
{
public:
    /**
     * Default constructor.
     *
     * \exception Error is thrown if a new LUA state cannot be created. This is usually an
     *            indication for an out-of-memory condition.
     */
    LuaState();

    /// Deleted copy constructor.
    LuaState(const LuaState& other) = delete;

    /**
     * Move constructor.
     * The new object takes over the LUA state from the original one. The moved-from
     * object can be deleted safely but other operations on it may invoke undefined
     * behavior.
     */
    LuaState(LuaState&& other);

    /// Destructor: Close the LUA state.
    ~LuaState() noexcept;

    /**
     * Assign a value to the field with a given key in the table at the specified stack
     * index.
     *
     * This call is equivalent to the LUA statement "table[key] = value", including the
     * calling of metamethods. The call does not change the stack position.
     *
     * \param key                The key or table index to which the value should be
     *                           assigned.
     * \param value              The value to be assigned.
     * \param table_stack_index  The LUA stack index of the table in which the assignment
     *                           should take place. The default stack index of -1 refers
     *                           to the element at the top of the stack.
     *
     * \exception Error is thrown if a zero stack index is given (which is always illegal
     *            in LUA).
     */
    template <typename KeyType, typename ValueType>
    void assign_field(KeyType key, ValueType value, int table_stack_index = -1)
    {
        if (table_stack_index < 0)
            table_stack_index -= 2; // adjust relative stack indices by the two elements we are about to push
        else if (table_stack_index == 0)
            throw Error("Zero stack index in assign_field()");

        push(key);
        push(value);
        set_table(table_stack_index);
    }

    /**
     * Execute the LUA function on top of the stack, passing the given arguments.
     *
     * The call removes the function from the stack and leaves all output parameters on
     * it.
     *
     * \returns the number of function results pushed onto the stack.
     *
     * \exception Error is thrown if there is no function on the top of the LUA stack or
     *            if the function call causes a runtime error. In the latter case, the
     *            function is popped from the stack and no additional values are left on
     *            it.
     */
    template <typename... ArgumentTypes>
    int call_function(const ArgumentTypes&... args)
    {
        const int initial_stack_pos = get_top();

        call_function_with_n_arguments(sizeof...(ArgumentTypes), args...);

        return get_top() - initial_stack_pos + 1; // +1: function was popped from stack
    }

    /**
     * Create an empty table and push it onto the LUA stack.
     *
     * Although LUA's memory management is fully automatic, it is sometimes useful to
     * preallocate some space in the table for performance reasons. This can be done with
     * the following function parameters:
     *
     * \param num_seq_elements   A hint for how many sequential ("array") elements should
     *                           be preallocated.
     * \param num_other_elements A hint for how many other elements should be
     *                           preallocated.
     *
     * \exception Error is thrown if the table cannot be created (e.g. if the hints for
     *            the number of elements are negative).
     */
    void create_table(int num_seq_elements = 0, int num_other_elements = 0);

    /**
     * Return a pointer to the LUA state.
     * This pointer is null if the LUA state has been closed. Otherwise, it can be used to
     * call functions from the LUA C library directly. Calling lua_close() on it leads to
     * undefined behavior.
     */
    lua_State* get() const noexcept { return state_; }

    /**
     * Retrieve the global variable with the specified name, push its value onto the LUA
     * stack and return its type.
     */
    LuaType get_global(const std::string& global_var_name);

    /**
     * Return the number of elements on the LUA stack.
     * Zero means that the stack is empty.
     */
    int get_top();

    /**
     * Return the type of the value at the given stack index.
     *
     * \param stack_index  A LUA stack index; by default (-1), the type of the topmost
     *                     value on the stack is returned.
     *
     * \returns the type of the value at the specified stack index or LuaType::none if the
     *          stack index is invalid.
     */
    LuaType get_type(int stack_index = -1);

    /**
     * Load a LUA script from a string without running it.
     * The script is precompiled into a chunk and its syntax is checked.
     * \exception Error is thrown if a syntax error is found or if there is insufficient
     *            memory.
     */
    void load_string(const std::string& script);

    /// Copy assignment is deleted.
    LuaState& operator=(const LuaState& other) = delete;

    /**
     * Move assignment.
     * The existing LUA state is closed and the assigned one is taken over. The moved-from
     * state can be deleted safely but other operations on it may invoke undefined
     * behavior.
     */
    LuaState& operator=(LuaState&& other) noexcept;

    /**
     * Push a value onto the LUA stack.
     * This function is overloaded for several C++ data types. Floating-point values are
     * stored on the stack as LUA numbers, integers as LUA integers, strings as LUA
     * strings.
     */
    template <typename FloatType,
              std::enable_if_t<std::is_floating_point<FloatType>::value, bool> = true>
    void push(FloatType number)
    {
        push_number(number);
    }

    /// \overload
    template <typename IntType,
              std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
    void push(IntType integer)
    {
        push_integer(integer);
    }

    /// \overload
    void push(const char* str) { push_string(str); }

    /// \overload
    void push(const std::string& str) { push_string(str); }

    /// Push an integer onto the LUA stack.
    void push_integer(long long value);

    /// Push a number onto the LUA stack.
    void push_number(double value);

    /**
     * Push a string onto the LUA stack.
     * If a null pointer is given, a nil value is pushed onto the stack instead.
     */
    void push_string(const char* str);

    /// Push a string onto the LUA stack.
    void push_string(const std::string& str);

    /**
     * Pop an integer from the LUA stack and return it.
     *
     * \exception Error is thrown if the topmost value on the stack cannot be converted to
     *            an integer. In this case, the stack position is not modified.
     */
    long long pop_integer();

    /**
     * Pop a number from the LUA stack and return it.
     *
     * \exception Error is thrown if the topmost value on the stack cannot be converted
     *            into a number. In this case, the stack position is not modified.
     */
    double pop_number();

    /**
     * Pop a string from the LUA stack and return it.
     *
     * \exception Error is thrown if the topmost value on the stack cannot be converted
     *            into a string. In this case, the stack position is not modified.
     */
    std::string pop_string();

    /**
     * Pops a value from the LUA stack and assigns it to the global variable with the
     * specified name.
     */
    void set_global(const std::string& global_var_name);

    /**
     * Assign a table entry.
     *
     * This function does the equivalent of the LUA command "table[key] = value",
     * including the calling of metamethods, if any. The table is identified by its index
     * on the LUA stack:
     *
     * \param table_stack_index  LUA stack index of the table to be assigned to
     *
     * Both the key and the value are taken from the stack: The value is the topmost
     * element (index -1), the key the second-topmost (index -2). Both elements are popped
     * from the stack after the assignment.
     *
     * \warning
     * Calling this function without having a key and a value on the LUA stack causes
     * undefined behavior. Consider using assign_field() instead.
     */
    void set_table(int table_stack_index);

private:
    lua_State* state_ = nullptr;

    void call_function_with_n_arguments(int num_args);

    template <typename ArgType>
    void call_function_with_n_arguments(int num_args, const ArgType& arg)
    {
        push(arg);
        call_function_with_n_arguments(num_args);
    }

    template <typename FirstArgType, typename... OtherArgTypes>
    void call_function_with_n_arguments(int num_args,
        const FirstArgType& first_arg, const OtherArgTypes&... other_args)
    {
        push(first_arg);
        call_function_with_n_arguments(num_args, other_args...);
    }

    /*
     * Close the LUA state (bring LuaState into a moved-from state).
     * After closing, most member functions will invoke undefined behavior when called.
     * The object can be safely deleted, however. The function may be called on an already
     * closed object.
     */
    void close() noexcept;
};


} // namespace avto


#endif
