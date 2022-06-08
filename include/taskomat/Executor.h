/**
 * \file   Executor.h
 * \author Lars Froehlich
 * \date   Created on May 30, 2022
 * \brief  Declaration of the Executor class.
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

#ifndef TASKOMAT_EXECUTOR_H_
#define TASKOMAT_EXECUTOR_H_

#include <future>
#include <memory>
#include "taskomat/Context.h"
#include "taskomat/Message.h"
#include "taskomat/Sequence.h"

namespace task {

/**
 * An executor runs a copy of a given Sequence in a separate thread, receives messages
 * from it, and updates the local copy accordingly.
 *
 * A sequence is started in a separate thread with run_asynchronously(). Afterwards, the
 * main thread must periodically call update() to process messages from the thread. The
 * thread has finished when update() returns false.
 *
 * \code {.cpp}
 * Executor ex;
 * Sequence sequence = load_sequence();
 * Context context;
 *
 * // Start executing a copy of the sequence in a separate thread
 * ex.run_asynchronously(sequence, context);
 *
 * // Periodically call update() to bring our local copy of the sequence in sync with the
 * // other thread: This updates timestamps and "under execution" flags. Once the sequence
 * // has finished, update() returns false and the thread is joined automatically.
 * while (ex.update(sequence))
 *     sleep(0.1s);
 * \endcode
 *
 * \note
 * Calling update() in the main thread is mandatory to ensure that the sequence in the
 * worker thread can make progress. This is because the message queue for communication
 * between the threads has only a limited capacity, and execution is paused once it is
 * full. Only calls to update() take messages out of the queue again.
 */
class Executor
{
public:
    /// Construct an Executor.
    Executor();

    /**
     * Determine if the executor is currently running a sequence in a separate thread.
     *
     * \returns true if a sequence is being executed or false otherwise.
     */
    bool is_busy();

    /**
     * Start a copy of the given sequence in a separate thread.
     * The given sequence is updated in this thread whenever update() is called.
     *
     * \param sequence  The sequence to be executed
     * \param context   The context in which the sequence should be executed.
     *
     * \exception Error is thrown if this executor is still busy running another sequence.
     */
    void run_asynchronously(Sequence sequence, Context context);

    /**
     * Update the local copy of the sequence from messages that have arrived from the
     * execution thread.
     *
     * \param sequence  Reference to the local copy of the sequence that was started with
     *                  run_asynchronously()
     *
     * \returns true if the sequence is still being executed or false otherwise. The
     *          return value is identical to is_busy().
     */
    bool update(Sequence& sequence);

private:
    /**
     * Shared message queue for the main thread (reader) and the execution thread
     * (writer). The queue must stay at a fixed address so both threads can access it even
     * if the Executor object is moved.
     */
    std::shared_ptr<MessageQueue> queue_;

    /**
     * A future holding the results of the execution thread.
     *
     * At the moment, there are no results (hence void), but the future is still useful
     * to detect when the execution has finished.
     */
    std::future<void> future_;

    /// The function that is run in the execution thread.
    static void execute_sequence(Sequence sequence, Context context,
                                 std::shared_ptr<MessageQueue> queue) noexcept;
};

} // namespace task

#endif
