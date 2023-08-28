/**
 * \file   exceptions.h
 * \author Lars Fröhlich
 * \date   Created on July 4, 2017
 * \brief  Definition of the Error exception class.
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

#ifndef TASKOLIB_EXCEPTIONS_H_
#define TASKOLIB_EXCEPTIONS_H_

#include <cstring>
#include <stdexcept>
#include <string>

#include "taskolib/StepIndex.h"

namespace task {

/**
 * An exception class carrying an error message and, optionally, the index of the step in
 * which the error occurred.
 *
 * Error is used as the standard exception class by many functions throughout Taskolib.
 * It can be used directly or inherited from.
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
 *
 * try
 * {
 *     throw task::Error("An error has occurred", 42);
 * }
 * catch (const task::Error& e)
 * {
 *     std::cerr << e.what();
 *     auto maybe_step_index = e.get_index();
 *     if (maybe_step_index)
 *         std::cerr << ": step index " << *maybe_step_index;
 *     std::cerr << "\n";
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
    explicit Error(const std::string& msg, OptionalStepIndex opt_step_index = gul14::nullopt)
        : std::runtime_error(msg)
        , index_{ opt_step_index }
    {}

    explicit Error(const char* msg, OptionalStepIndex opt_step_index = gul14::nullopt)
        : std::runtime_error(msg)
        , index_{ opt_step_index }
    {}

    /// Return the associated step index.
    OptionalStepIndex get_index() const { return index_; }

    /// Determine if two Error objects have the same content.
    friend bool operator==(const Error& lhs, const Error& rhs) noexcept
    {
        return (std::strcmp(lhs.what(), rhs.what()) == 0)
            && lhs.index_ == rhs.index_;
    }

    /// Determine if two Error objects have different content.
    friend bool operator!=(const Error& lhs, const Error& rhs) noexcept
    {
        return !(lhs == rhs);
    }

private:
    OptionalStepIndex index_;
};

} // namespace task

#endif
