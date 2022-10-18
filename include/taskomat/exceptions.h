/**
 * \file   exceptions.h
 * \author Lars Froehlich
 * \date   Created on July 4, 2017
 * \brief  Definitions of the Error, ErrorAtIndex exception classes and of the StepIndex
 *         type.
 *
 * \copyright Copyright 2017-2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOMAT_EXCEPTIONS_H_
#define TASKOMAT_EXCEPTIONS_H_

#include <cstdint>
#include <stdexcept>
#include <string>

namespace task {

/// A type for storing the index of a Step in a Sequence.
using StepIndex = std::uint16_t;

/**
 * A generic exception class carrying a message string.
 *
 * The Error class is used as the standard exception by many functions throughout the
 * Taskomat library. It can be used directly or inherited from.
 *
 * \code
 * try
 * {
 *     throw task::Error("An error has occurred");
 * }
 * catch (const task::Error& e)
 * {
 *     std::cerr << e.what() << "\n";
 * }
 * \endcode
 *
 * \note
 * task::Error is derived from std::runtime_error. It can therefore be caught by
 * `catch (const std::exception&)`, `catch (const std::runtime_error&)`, and
 * `catch (const task::Error&)`.
 */
class Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/**
 * An exception class storing the index of the step in which an error occurred, in
 * addition to the error message.
 *
 * \code
 * try
 * {
 *     throw task::ErrorAtIndex("An error has occurred", 42);
 * }
 * catch (const task::ErrorAtIndex& e)
 * {
 *     std::cerr << e.what() << " in step " << e.get_index() << "\n";
 * }
 * \endcode
 *
 * \note
 * task::ErrorAtIndex inherits from std::runtime_error. It can therefore be caught by
 * `catch (const std::exception&)`, `catch (const std::runtime_error&)`,
 * `catch (const task::Error&)`, and `catch (const task::ErrorAtIndex&)`.
 */
class ErrorAtIndex : public Error
{
public:
    ErrorAtIndex(const std::string& msg, StepIndex index)
        : Error(msg)
        , index_(index)
    {}

    ErrorAtIndex(const char* msg, StepIndex index)
        : Error(msg)
        , index_(index)
    {}

    /// Return the associated step index.
    StepIndex get_index() const noexcept { return index_; }

private:
    StepIndex index_ = 0;
};

} // namespace task

#endif
