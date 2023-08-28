/**
 * \file   Context.h
 * \author Lars Fröhlich
 * \date   Created on December 20, 2021
 * \brief  Declaration of the Context and VariableValue types.
 *
 * \copyright Copyright 2021-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_CONTEXT_H_
#define TASKOLIB_CONTEXT_H_

#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

#include "sol/sol.hpp"
#include "taskolib/CommChannel.h"
#include "taskolib/default_message_callback.h"
#include "taskolib/Message.h"
#include "taskolib/StepIndex.h"
#include "taskolib/VariableName.h"

namespace task {

/**
 * Lua uses certain types to interface with C++.
 *
 * If these types are used the data comes from or shall go to the Lua engine.
 */
using LuaInteger = LUA_INTEGER; ///< The integer type used by the Lua interpreter
using LuaFloat = LUA_NUMBER; ///< The floating point type used by the Lua interpreter
using LuaString = std::string; ///< The string type used by the Lua interpreter
using LuaBool = bool; ///< The boolean type used by the Lua interpreter

/**
 * The types available to forward variables from one Step to the next.
 */
using VarInteger = long long; ///< Storage type for integral numbers
using VarFloat = double; ///< Storage type for floatingpoint number
using VarString = std::string; ///< Storage type for strings
using VarBool = bool; ///< Storage type for booleans

/**
 * A VariableValue is a variant over all Variable types.
 *
 * Variable names are associated with these values via a map in the Context class.
 *
 * Be careful when assigning a string to a VariableValue:
 * Do not use a char* to pass the string, it might be converted to bool instead
 * of the expected std::string. The conversion depends on the used compiler (version).
 */
using VariableValue = std::variant<
    VarInteger,
    VarFloat,
    VarString,
    VarBool>;

/**
 * Associative table that holds Lua variable names and their value.
 *
 * The keys are of type \a VariableName and the values \a VariableValue.
 */
using VariableTable = std::unordered_map<VariableName, VariableValue>;

/**
 * A message callback function receives a Message object as a parameter. It is called on
 * the main thread whenever a message is being processed.
 */
using MessageCallback = std::function<void(const Message&)>;

/**
 * A context stores information that influences the execution of steps and sequences,
 * namely:
 * - A list of variables that can be im-/exported into steps.
 * - An initialization function (in C++) that is called on a Lua state before a step is
 *   executed.
 * - A step setup script (in Lua) that is executed before a step is executed. Global
 *   variables and functions defined in this step setup script can be accessed by the Lua
 *   script in the step. When a sequence is executed, this step setup script is
 *   overwritten with the one from the sequence.
 * - A callback that is invoked whenever a message is being processed by the execution
 *   engine (see below for details).
 *
 * <h3>Message callback function</h3>
 *
 * The Context contains a message_callback_function that is called once for each message
 * that is being processed by the execution engine in the main thread. By default, this
 * function will send the output of the Lua "print()" command and error messages to
 * stdout (see default_message_callback()). However, message_callback_function can also be
 * set to nullptr (to disable any special behavior) or to a custom function:
 * \code
 * void my_message_callback(const Message& msg)
 * {
 *     if (msg.get_type() == Message::Type::output)
 *         send_to_console(msg.get_text());
 *     else if (msg.get_type() == Message::Type::sequence_stopped_with_error)
 *         show_alert_box(msg.get_text());
 * }
 *
 * Context context;
 * context.message_callback_function = my_message_callback;
 * \endcode
 * The callback function receives a reference to the message that is being processed.
 * If a sequence is executed in the current thread (e.g. via Sequence::execute()), the
 * callback is executed whenever a message is generated. If the sequence is handed to an
 * Executor for parallel execution, the callback is run whenever the main thread has
 * received the message (i.e. typically within Executor::update()). Callbacks are never
 * executed on the worker thread.
 */
struct Context
{
    /// A map of variables (names and values) that can be im-/exported into steps.
    VariableTable variables;

    /// Step setup script with common functions or constants like a small library.
    /// Overwritten when a sequence is started.
    std::string step_setup_script = "";

    /// An initialization function that is called on a Lua state before a step is executed.
    std::function<void(sol::state&)> step_setup_function;

    /**
     * A callback (or "hook") function that is invoked whenever a message is processed
     * during the execution of a sequence.
     */
    MessageCallback message_callback_function = default_message_callback;
};

} // namespace task

#endif
