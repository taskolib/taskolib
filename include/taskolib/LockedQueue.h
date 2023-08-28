/**
 * \file   LockedQueue.h
 * \author Lars Fröhlich, Marcus Walla
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

#ifndef TASKOLIB_LOCKEDQUEUE_H_
#define TASKOLIB_LOCKEDQUEUE_H_

#include <condition_variable>
#include <mutex>
#include <gul14/optional.h>
#include <gul14/SlidingBuffer.h>

namespace task {

/**
 * A thread-safe locking message queue.
 *
 * Messages are added to the end of the queue with push() and taken from the start of the
 * queue with pop(). These calls block if the queue is full or if no messages are
 * available, respectively. try_push() and try_pop() are non-blocking alternatives.
 *
 * \code
 * // Set up queue with capacity for 10 integers
 * LockedQueue<int> queue{ 10 };
 *
 * // Start sender thread, pushing 100 integers into the queue
 * std::thread sender([&queue]()
 *     {
 *         for (int i = 1; i <= 100; ++i)
 *             queue.push(i);
 *     });
 *
 * // Pull all 100 integers out of the queue in the main thread
 * for (int i = 1; i <= 100; ++i)
 * {
 *     int val = queue.pop();
 *     assert(val == i);
 * }
 *
 * // Join the sender thread
 * sender.join();
 * \endcode
 */
template <typename MessageT>
class LockedQueue
{
public:
    using MessageType = MessageT;
    using message_type = MessageT;
    using SizeType = std::uint32_t;
    using size_type = SizeType;

    /// Construct a queue that is able to hold a given maximum number of entries.
    LockedQueue(SizeType capacity)
        : queue_(capacity)
    { }

    /// Return the maximal number of entries in the queue.
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
        std::unique_lock<std::mutex> lock(mutex_);

        if (queue_.empty())
            cv_message_available_.wait(lock, [this] { return not queue_.empty(); });

        auto msg = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        cv_slot_available_.notify_one();
        return msg;
    }

    /**
     * Fetch the last message pushed to the queue and returns a copy of it. It will not be
     * removed from the queue.
     *
     * This call blocks until a message is available.
     */
    MessageType back() const
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (queue_.empty())
            cv_message_available_.wait(lock, [this] { return not queue_.empty(); });

        auto msg = queue_.back();
        lock.unlock();
        return msg;
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
        std::unique_lock<std::mutex> lock(mutex_);

        if (queue_.filled())
            cv_slot_available_.wait(lock, [this] { return not queue_.filled(); });

        queue_.push_back(std::forward<MsgT>(msg));
        lock.unlock();
        cv_message_available_.notify_one();
    }

    /// Return the number of messages in the queue.
    SizeType size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    /**
     * Remove a message from the front of the queue and return it.
     *
     * This call blocks until a message is available.
     *
     * \see try_pop()
     */
    gul14::optional<MessageType> try_pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (queue_.empty())
            return gul14::nullopt;

        auto msg = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        cv_slot_available_.notify_one();
        return msg;
    }

    /**
     * Try to insert a message at the end of the queue.
     *
     * This call returns true if the message was successfully enqueued or false if the
     * queue temporarily had no space to store the message.
     *
     * Messages given as an rvalue are only moved from if they can actually be inserted
     * into the queue.
     */
    template <typename MsgT,
              std::enable_if_t<std::is_convertible_v<MsgT, MessageType>, bool> = true>
    bool try_push(MsgT&& msg)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (queue_.filled())
            return false;

        queue_.push_back(std::forward<MsgT>(msg));
        lock.unlock();
        cv_message_available_.notify_one();
        return true;
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
