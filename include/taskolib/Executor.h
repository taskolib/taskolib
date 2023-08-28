/**
 * \file   Executor.h
 * \author Lars Fröhlich
 * \date   Created on May 30, 2022
 * \brief  Declaration of the Executor class.
 *
 * \copyright Copyright 2022-2023 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef TASKOLIB_EXECUTOR_H_
#define TASKOLIB_EXECUTOR_H_

#include <future>
#include <memory>

#include "taskolib/CommChannel.h"
#include "taskolib/Context.h"
#include "taskolib/Sequence.h"
#include "taskolib/StepIndex.h"

namespace task {

/**
 * An executor runs a copy of a given Sequence (or just a single step within it) in a
 * separate thread, receives messages from it, and updates the local instance of the
 * Sequence accordingly.
 *
 * A sequence is started in a separate thread with run_asynchronously() and a single step
 * can be started in isolation with run_single_step_asynchronously(). Afterwards, the
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

    // Not copyable but movable (you can't copy a future)
    Executor(Executor const&) = delete;
    Executor& operator=(Executor const&) = delete;
    Executor(Executor&&) = default;
    Executor& operator=(Executor&&) = default;

    ~Executor() { cancel(); };

    /**
     * Terminate a running sequence.
     *
     * If a sequence is running in a separate thread, this call sends a termination
     * request and waits for the thread to shut down. If no sequence is currently running,
     * the call has no effect.
     * An associated Sequence will not be updated and all messages are lost.
     * Usually you should call \ref cancel(Sequence& sequence).
     */
    void cancel();

    /**
     * Terminate a running sequence.
     *
     * If a sequence is running in a separate thread, this call sends a termination
     * request and waits for the thread to shut down. If no sequence is currently running,
     * the call has no effect.
     * The associated Sequence will be updated with all pending messages.
     */
    void cancel(Sequence& sequence);

    /**
     * Start a copy of the given sequence in a separate thread.
     * The given sequence is updated in this thread whenever update() is called.
     *
     * \param sequence  Reference to the sequence to be executed; This sequence is marked
     *                  as is_running(), but the actual execution takes place on a copy
     *                  in a separate thread.
     * \param context   The context in which the sequence should be executed.
     *
     * \exception Error is thrown if this executor is still busy running another sequence.
     */
    void run_asynchronously(Sequence& sequence, Context context);

    /**
     * Start a single step of the given sequence in a separate thread.
     * The given sequence is updated in this thread whenever update() is called.
     *
     * \param sequence    Reference to the sequence containing the step to be executed;
     *                    This sequence is marked as is_running(), but the actual
     *                    execution takes place on a copy in a separate thread.
     * \param context     The context in which the step should be executed
     * \param step_index  The index of the step to be executed
     *
     * \exception Error is thrown if this executor is still busy running another sequence
     *            or if the step index is invalid.
     */
    void run_single_step_asynchronously(Sequence& sequence, Context context,
                                        StepIndex step_index);

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

    /**
     * Retrieve the variables stored in the context.
     *
     * After a sequence has run the context variables can be retrieved
     * for inspection. Note that the variables are only updated after the
     * Sequence has stopped.
     * Use this only after update() returns false (is_busy() == false).
     *
     * \returns the context variable mapping.
     */
    VariableTable get_context_variables() { return context_.variables; }

private:
    /**
     * Communications channel between the main thread and the executing thread.
     *
     * It must stay at a fixed address so both threads can access it even if the Executor
     * object is moved. Both the main thread and the worker thread have one shared_ptr
     * to the channel.
     */
    std::shared_ptr<CommChannel> comm_channel_;

    /**
     * A future for the result of the execution thread.
     * Once the thread has joined, it contains the context variables from the executed
     * sequence.
     */
    std::future<VariableTable> future_;

    /**
     * A local copy of the context that was used to start the last sequence.
     * Its output callbacks are used to produce "console" and logging output.
     * After the Sequence finished it contains the processed context.
     */
    Context context_;

    /**
     * Start a sequence- or single-step-execution function in a separate thread.
     *
     * \param sequence      The Sequence to be started or the parent sequence of the step
     * \param context       The execution Context
     * \param comm_channel  Shared pointer to a CommChannel for communication (can be
     *                      null)
     * \param step_index    For single-step execution, this is the index of the step to be
     *                      started; for sequence execution, it has no meaning.
     *
     * \exception Error is thrown if the executor is already busy. The function can also
     *            throw any exception from std::async if the thread cannot be created.
     */
    void launch_async_execution(Sequence& sequence, Context context,
                                OptionalStepIndex step_index);

    /**
     * Determine if the executor is currently running a sequence in a separate thread.
     *
     * \returns true if a sequence is being executed or false otherwise.
     *
     * \note
     * This function does not retrieve any messages from the message queue, but it does
     * join the worker thread if it has finished its sequence.
     */
    bool is_busy();
};

} // namespace task

#endif
