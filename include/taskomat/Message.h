/**
 * \file   Message.h
 * \author Lars Froehlich
 * \date   Created on April 1, 2022
 * \brief  Declaration of the Message class.
 *
 * \copyright Copyright 2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOMAT_MESSAGE_H_
#define TASKOMAT_MESSAGE_H_

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

#include <gul14/escape.h>

#include "taskomat/time_types.h"

namespace task {

/**
 * A message carrying some text, a timestamp, and a type, to be transported with a message
 * queue between threads.
 */
class Message
{
public:
    /// Type for the associated index (for convenience, same as Sequence::SizeType)
    using IndexType = std::uint16_t;

    /// The type of this message.
    enum class Type
    {
        output, ///< a message string that was output by a step via print()
        log_info, ///< a log entry of informational character
        log_warning, ///< a log entry carrying some kind of warning
        log_error, ///< a log entry carrying an error message
        sequence_started, ///< a sequence has been started
        sequence_stopped, ///< a sequence has stopped regularly
        sequence_stopped_with_error, ///< a sequence has been stopped because of an error
        step_started, ///< a step inside a sequence has been started
        step_stopped, ///< a step inside a sequence has stopped regularly
        step_stopped_with_error, ///< a step inside a sequence has been stopped because of an error
        undefined ///< marker for last type
    };

private:
    static constexpr std::array<char const*, static_cast<int>(Type::undefined) + 1> type_description_ =
    {
        "output",
        "log_info",
        "log_warning",
        "log_error",
        "sequence_started",
        "sequence_stopped",
        "sequence_stopped_with_error",
        "step_started",
        "step_stopped",
        "step_stopped_with_error",
        "undefined"
    };


public:
    /// Construct an empty message.
    Message() = default;

    /// Construct an initialized message from the given parameters.
    Message(Type type, std::string text, TimePoint timestamp, IndexType index)
        : text_{ std::move(text) }
        , timestamp_{ timestamp }
        , type_{ type }
        , index_{ index }
    {}

    /// Return the associated index.
    IndexType get_index() const noexcept { return index_; }

    /**
     * Return the message text.
     *
     * This function returns a reference to a member variable. Be aware of the associated
     * lifetime implications!
     */
    const std::string& get_text() const { return text_; }

    /// Return the message type.
    Type get_type() const noexcept { return type_; }

    /// Return the timestamp.
    TimePoint get_timestamp() const { return timestamp_; };

    /// Set the associated index.
    void set_index(IndexType index) { index_ = index; }

    /// Set the message text.
    void set_text(const std::string& text) { text_ = text; }

    /// Set the timestamp.
    void set_timestamp(TimePoint timestamp) { timestamp_ = timestamp; };

    /// Set the message type.
    void set_type(Type type) noexcept { type_ = type; }

    friend std::ostream& operator<<(std::ostream& stream, Type const& t) {
        stream << Message::type_description_[static_cast<int>(t)];
        return stream;
    }

    friend std::ostream& operator<<(std::ostream& stream, Message const& mess) {
        stream << "Message{ "
            << mess.index_
            << ": "
            << mess.type_
            << " \""
            << gul14::escape(mess.text_)
            << "\" "
            << dump_timepoint(mess.timestamp_)
            << " }\n";
        return stream;
    };

private:
    std::string text_;
    TimePoint timestamp_{};
    Type type_{ Type::output };
    IndexType index_{ 0 };
};

} // namespace task

#endif
