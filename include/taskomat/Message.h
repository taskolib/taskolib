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

#include <memory>
#include "taskomat/LockedQueue.h"
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
        log, ///< a message string for logging purposes
        sequence_started, ///< a sequence has been started
        sequence_stopped, ///< a sequence has stopped regularly
        sequence_stopped_with_error, ///< a sequence has been stopped because of an error
        step_started, ///< a step inside a sequence has been started
        step_stopped, ///< a step inside a sequence has stopped regularly
        step_stopped_with_error ///< a step inside a sequence has been stopped because of an error
    };

    /// Construct an empty message.
    Message() = default;

    /// Construct a message with a given timestamp.
    Message(Type type, std::string text, TimePoint timestamp)
        : text_{ std::move(text) }
        , timestamp_{ timestamp }
        , type_{ type }
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

private:
    std::string text_;
    TimePoint timestamp_{};
    Type type_{ Type::log };
    IndexType index_{ 0 };
};

/// A thread-safe queue holding Message objects.
using MessageQueue = LockedQueue<Message>;


/**
 * Create a new message on the heap and enqueue in the given message queue.
 *
 * \param queue      Pointer to the message queue. If this is null, the function does
 *                   nothing.
 * \param type       Message type (see Message::Type)
 * \param text       Message text
 * \param timestamp  Timestamp of the message
 *
 * \code
 * send_message<StepStartedMessage>(queue, timestamp);
 * // ... is equivalent to:
 * if (queue)
 *     queue->push(std::make_unique<StepStartedMessage>(timestamp));
 * \endcode
 */
inline
void send_message(MessageQueue* queue, Message::Type type, std::string text,
                  TimePoint timestamp)
{
    if (queue == nullptr)
        return;

    queue->push(Message(type, std::move(text), timestamp));
}


} // namespace task

#endif
