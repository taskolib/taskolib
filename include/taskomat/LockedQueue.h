/**
 * \file   LockedQueue.h
 * \author Lars Froehlich
 * \date   Created on April 1, 2022
 * \brief  Declaration of the LockedQueue class.
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

#ifndef TASKOMAT_LOCKEDQUEUE_H_
#define TASKOMAT_LOCKEDQUEUE_H_

#include <condition_variable>
#include <mutex>
#include <gul14/SlidingBuffer.h>

namespace task {

/**
 * A thread-safe locking message queue.
 *
 * Messages are added to the end of the queue with push() and taken from the start of the
 * queue with pop().
 */
template <typename MessageT>
class LockedQueue
{
public:
    using MessageType = MessageT;
    using message_type = MessageT;
    using SizeType = std::uint32_t;
    using size_type = SizeType;

    /// Construct a message queue that is able to hold a maximum of max_messages messages.
    LockedQueue(SizeType max_messages = 32u)
        : queue_(max_messages)
    { }

    /// Return the maximal number of messages in the queue.
    SizeType capacity() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.capacity();
    }

    /// Determine whether the queue is empty.
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    /**
     * Remove a message from the front of the queue and return it.
     *
     * This call blocks until a message is available.
     *
     * \see try_pop()
     */
    MessageType pop()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (queue_.empty())
                cv_message_available_.wait(lock, [this] { return not queue_.empty(); });

            if (not queue_.empty())
            {
                auto msg_ptr = std::move(queue_.front());
                queue_.pop_front();
                lock.unlock();
                cv_slot_available_.notify_one();
                return msg_ptr;
            }
        }
    }

    /**
     * Insert a message at the end of the queue.
     *
     * This call blocks until the queue has a free slot for the message.
     */
    template <typename MsgT,
              std::enable_if_t<std::is_convertible_v<MsgT, MessageType>, bool> = true>
    void push(MsgT&& msg)
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (queue_.filled())
                cv_slot_available_.wait(lock, [this] { return not queue_.filled(); });

            if (not queue_.filled())
            {
                queue_.push_back(std::forward<MsgT>(msg));
                lock.unlock();
                cv_message_available_.notify_one();
                return;
            }
        }
    }

    /// Return the number of messages in the queue.
    SizeType size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    /// Mutex protecting all member variables
    mutable std::mutex mutex_;

    /// Condition variable, triggered when at least one message has been added to the queue.
    mutable std::condition_variable cv_message_available_;

    /// Condition variable, triggered when at least one slot in the queue has been freed.
    mutable std::condition_variable cv_slot_available_;

    gul14::SlidingBuffer<MessageType> queue_;
};

} // namespace task

#endif
