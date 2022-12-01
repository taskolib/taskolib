/**
 * \file   Sequence.h
 * \author Marcus Walla, Lars Froehlich
 * \date   Created on February 8, 2022
 * \brief  A sequence of Steps.
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

#ifndef TASKOLIB_SEQUENCE_H_
#define TASKOLIB_SEQUENCE_H_

#include <algorithm>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#include <gul14/cat.h>
#include <gul14/finalizer.h>
#include <gul14/string_view.h>

#include "taskolib/CommChannel.h"
#include "taskolib/Context.h"
#include "taskolib/exceptions.h"
#include "taskolib/Step.h"
#include "taskolib/StepIndex.h"

namespace task {

/**
 * A sequence of \ref task::Step "Step"s to be executed under a given Context.
 *
 * On executing a validation is performed to check if the steps are consistent. When a
 * fault is detected an Error is thrown including a precise error message about what
 * fails.
 *
 * To modify the sequence the following member functions are implemented:
 *
 * -# push_back(): add a new Step at the end
 * -# pop_back(): remove a Step from the end
 * -# insert(): insert a Step at an arbitrary position
 * -# assign(): assign a new Step to an existing element
 * -# erase(): remove a Step or a range of steps
 * -# modify(): modify a Step inside the sequence via a function or function object
 *
 * \code {.cpp}
 * Sequence seq;
 * seq.push_back(Step{Step::type_try});
 * seq.push_back(Step{Step::type_action});
 * seq.push_back(Step{Step::type_catch});
 * seq.push_back(Step{Step::type_action});
 * seq.push_back(Step{Step::type_end});
 * // insert an action step at index 1
 * seq.insert(seq.begin() + 1, Step{Step::type_action});
 * \endcode
 *
 * Global variables and functions defined in the setup script can be accessed in the
 * script.
 *
 * The setup is only executed for Step types ACTION, IF, ELSEIF, or WHILE.
 */
class Sequence
{
public:
    /// Alias for step type.
    using SizeType = StepIndex;
    /// Alias for a vector iterator.
    using Iterator = std::vector<Step>::iterator;
    /// Alias for a constant vector iterator.
    using ConstIterator = std::vector<Step>::const_iterator;
    /// Alias for a constant reverse vector iterator.
    using ConstReverseIterator = std::vector<Step>::const_reverse_iterator;

    /// Max number of bytes of a Sequence label.
    static constexpr std::size_t max_label_length = 128;

    /**
     * Construct a Sequence with a descriptive name.
     * The label should describe the function of the sequence clearly and concisely.
     * The label must not be empty. Leading and trailing whitespaces will be removed.
     *
     * \param label descriptive and expressive label.
     *
     * \exception Error is thrown if the label is empty or if its length exceeds
     *            max_label_length bytes.
     */
    explicit Sequence(gul14::string_view label);

    /**
     * Assign a Step to the sequence entry at the given position.
     *
     * @param iter Position to which the Step should be assigned
     * @param step New step to be assigned to the sequence entry
     */
    void assign(ConstIterator iter, const Step& step);

    /**
     * Assign a Step to the sequence entry at the given position.
     *
     * @param iter Position to which the Step should be assigned
     * @param step New step to be assigned to the sequence entry
     */
    void assign(ConstIterator iter, Step&& step);

    /**
     * Return a constant iterator to the first Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstIterator begin() const noexcept { return steps_.cbegin(); }

    /**
     * Return a constant iterator to the first Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstIterator cbegin() const noexcept { return steps_.cbegin(); }

    /**
     * Return a constant iterator pointing past the last Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstIterator cend() const noexcept { return steps_.cend(); }

    /**
     * Return a constant reverse iterator to the last Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstReverseIterator crbegin() const noexcept { return steps_.crbegin(); }

    /**
     * Return a constant reverse iterator pointing before the first Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstReverseIterator crend() const noexcept { return steps_.crend(); }

    /**
     * Validates if the \a Step 's token are correctly enclosed in a proper way.
     *
     * It is done by validating the step types where each must fit to one of the
     * following conditions:
     *
     * -# each type \a Step::type_try must have the corresponding
     *    \a Step::type_catch and \a Step::type_end
     * -# each type \a Step::type_if must have n-times \a Step::type_elseif and/or
     *  \a Step::type_else with a tailing \a Step::type_end, n >= 0.
     * -# each type \a Step::while must have the corresponding \a Step::type_end
     *
     * As a body of each surrounding token must have at least one \a Step::type_action
     * token.
     *
     * If one of those is ill-formed an \a Error exception is thrown.
     */
    void check_syntax() const;

    /// Determine whether the sequence contains no steps.
    bool empty() const noexcept { return steps_.empty(); }

    /**
     * Return a constant iterator pointing past the last Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstIterator end() const noexcept { return steps_.cend(); }

    /**
     * Remove \a Step iterator from sequence.
     *
     * It returns the iterator after the removed one.
     *
     * All iterators, inclusive the \a end() iterator are invalid after this operation.
     *
     * @param iter \a Step iterator to be removed
     * @return Steps::iterator iterator after the removed \a Step
     */
    ConstIterator erase(ConstIterator iter);

    /**
     * Remove a bunch of \a Step 's iterator from sequence from \a first to \a last . The
     * removed iterators is exclusive the \a last one: [first, last)
     *
     * \code
     * 0: ACTION
     * 1:    WHILE
     * 2:        ACTION
     * 3:    END
     * 4: ACTION
     * \endcode
     *
     * After removing iterator 1 to exclusive 4 ( \a while-loop ):
     *
     * \code
     * 0: ACTION
     * 1: ACTION
     * \endcode
     *
     * When you try to remove the \a last one as \a end() the first remove \a Step is
     * returned. Example: remove iterator 1 to 5 (here \a end() ):
     *
     * \code
     * 0: ACTION
     * 1:    WHILE
     * 2:        ACTION
     * 3:    END
     * 4: ACTION
     * \endcode
     *
     * Result: return iterator with step \a while (element 1)
     *
     * \code
     * 0: ACTION
     * \endcode
     *
     * All iterators, inclusive the \a end() iterator are invalid after this operation.
     *
     * @param first first \a Step iterator to be removed
     * @param last last \a Step iterator to be removed (exclusive)
     * @return Steps::iterator new iterator after erasing a bunch of \a Step 's
     */
    ConstIterator erase(ConstIterator first, ConstIterator last);

    /**
     * Execute the sequence within a given context.
     *
     * This function first performs a syntax check and throws an Error exception if it
     * fails. Then, it executes the sequence step by step, following the control flow
     * given by steps such as WHILE, IF, TRY, and so on. It returns when the sequence is
     * finished or has stopped with an error.
     *
     * During execute(), is_running() returns true to internal functions or LUA callbacks.
     *
     * \param context A context for storing variables that can be exchanged between
     *                different steps. The context may also contain a LUA init function
     *                that is run before each step.
     * \param comm_channel  Pointer to a communication channel. If this is a null pointer,
     *                no messages are sent and no external interaction with the running
     *                sequence is possible. Otherwise, messages for starting/stopping
     *                steps and the sequence itself are sent and termination requests are
     *                honored.
     *
     * \exception Error is thrown if the script cannot be executed due to a syntax error
     *            or if it raises an error during execution. In these cases, the error
     *            message is also stored in the Sequence object and can be retrieved with
     *            get_error_message().
     */
    void execute(Context& context, CommChannel* comm_channel);

    /**
     * Return a string explaining why the sequence stopped prematurely.
     *
     * If the sequence finished normally, the returned string is empty.
     *
     * \see execute()
     */
    const std::string& get_error_message() const noexcept { return error_message_; }

    /**
     * Return an error string if the sequence is not consistently nested, or an empty
     * string if the nesting is correct.
     */
    const std::string& get_indentation_error() const noexcept { return indentation_error_; }

    /**
     * Return the sequence label.
     *
     * @returns a descriptive name for the sequence.
     */
    const std::string& get_label() const noexcept { return label_; }

    /**
     * Insert the given Step into the sequence just before the specified iterator.
     *
     * This can trigger a reallocation that invalidates all iterators.
     *
     * \param iter  an iterator indicating the position before which the new step should
     *              be inserted
     * \param step  the Step to be inserted
     * \returns an iterator to the newly inserted Step
     * \exception Error is thrown if the Sequence has no capacity for additional entries.
     */
    ConstIterator insert(ConstIterator iter, const Step& step);

    /**
     * Insert the given Step into the sequence just before the specified iterator.
     *
     * This can trigger a reallocation that invalidates all iterators.
     *
     * \param iter  an iterator indicating the position before which the new step should
     *              be inserted
     * \param step  the Step to be inserted by moving
     * \returns an iterator to the newly inserted Step
     * \exception Error is thrown if the Sequence has no capacity for additional entries.
     */
    ConstIterator insert(ConstIterator iter, Step&& step);

    /**
     * Retrieve if the sequence is executed.
     *
     * \return true on executing otherwise false.
     */
    bool is_running() const noexcept { return is_running_; }

    /// Return the maximum number of steps that a sequence can hold.
    static SizeType max_size() noexcept { return std::numeric_limits<StepIndex>::max(); }

    /**
     * Modify a step inside the sequence.
     *
     * This function modifies one of the sequence steps in place. The modification is done
     * by a user-supplied function that receives a mutable reference to the Step indicated
     * via an iterator:
     * \code
     * Sequence seq = get_sequence_from_somewhere();
     * auto it = seq.begin(); // get an iterator to the Step that should be modified
     *
     * seq.modify(it, [](Step& step) { step.set_label("Modified step"); });
     * \endcode
     *
     * Background: A Sequence does not allow its steps to be modified directly from the
     * outside via references or iterators because it has to uphold certain class
     * invariants (like "all steps are always correctly indented"). A call to modify(),
     * however, reestablishes these invariants after the modification if necessary.
     *
     * \param iter  An iterator to the Step that should be modified. The step must be part
     *              of this sequence.
     * \param modification_fct  A function or function object with the signature
     *              `void fct(Step&)` that applies the desired modifications on a Step.
     *              The step reference becomes invalid after the call.
     *
     * \exception
     * If an exception is thrown by the modification function, the step may only be
     * partially modified, but the invariants of the sequence are maintained (basic
     * exception guarantee).
     */
    template <typename Closure>
    void modify(ConstIterator iter, Closure modification_fct)
    {
        throw_if_running();

        // Construct a mutable iterator from the given ConstIterator
        const auto it = steps_.begin() + (iter - steps_.cbegin());

        // Reindent at the end of the function, even if an exception is thrown
        auto indent_if_necessary = gul14::finally(
            [this,
             it,
             old_indentation_level = it->get_indentation_level(),
             old_type = it->get_type(),
             old_disabled = it->is_disabled()]()
            {
                if (it->get_type() != old_type
                    || it->get_indentation_level() != old_indentation_level)
                {
                    indent();
                }

                // Special treatment for re-enabling an entire block-with-continuation
                // if its head step is re-enabled
                if (it->is_disabled() == false && old_disabled == true)
                {
                    if (it->get_type() == Step::type_if
                        || it->get_type() == Step::type_while
                        || it->get_type() == Step::type_try)
                    {
                        auto it_end = find_end_of_continuation(it);
                        std::for_each(it, it_end, [](Step& st) { st.set_disabled(false); });
                    }
                }

                enforce_consistency_of_disabled_flags();
            });

        modification_fct(*it);
    }

    /**
     * Access the step at a given index.
     *
     * The index operator can only be used for read access to the sequence steps. This is
     * because Sequence has to maintain invariants such as the correct indentation
     * whenever steps are modified.
     */
    const Step& operator[](SizeType idx) const noexcept { return steps_[idx]; }

    /**
     * Remove the last element from the sequence.
     *
     * Calling pop_back() on an empty Sequence returns silently. Iterators and references to the
     * last element as well as the end() iterator are invalidated.
     */
    void pop_back();

    /**
     * Add a Step to the end of the sequence.
     *
     * \param step  the Step to be appended to the sequence
     * \exception Error is thrown if the Sequence has no capacity for additional entries.
     */
    void push_back(const Step& step);

    /**
     * Add a Step to the end of the sequence.
     *
     * \param step  the Step to be appended to the sequence by moving
     * \exception Error is thrown if the Sequence has no capacity for additional entries.
     */
    void push_back(Step&& step);

    /**
     * Return a constant reverse iterator to the last Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstReverseIterator rbegin() const noexcept { return steps_.crbegin(); }

    /**
     * Return a constant reverse iterator pointing before the first Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstReverseIterator rend() const noexcept { return steps_.crend(); }

    /**
     * Set an error message for the sequence.
     *
     * \param msg  A string explaining why the sequence stopped prematurely; to signalize
     *             that the sequence finished normally, it should be empty.
     *
     * \note
     * This is usually not a useful call for end users of the library. It is used by the
     * Executor class and by unit tests.
     */
    void set_error_message(gul14::string_view msg);

    /**
     * Set the sequence label.
     *
     * Leading and trailing whitespace is trimmed, and the resulting label must not be
     * empty.
     *
     * \param label  descriptive and expressive label.
     *
     * \exception Error is thrown if the label is empty or if its length exceeds
     *            max_label_length bytes.
     */
    void set_label(gul14::string_view label);

    /**
     * Set the sequence into the state "is running" (true) or "is not running" (false).
     *
     * \note
     * This is usually not a useful call for end users of the library. It is used by the
     * Executor class and by unit tests.
     */
    void set_running(bool running) noexcept { is_running_ = running; }

    /// Return the number of steps contained in this sequence.
    SizeType size() const noexcept { return static_cast<SizeType>(steps_.size()); }

private:
    /**
     * A string explaining why the sequence stopped prematurely (empty if it finished
     * normally).
     */
    std::string error_message_;

    /// Empty if indentation is correct and complete, error message otherwise
    std::string indentation_error_;

    /// Sequence label.
    std::string label_;

    /// Collection of steps.
    std::vector<Step> steps_;

    /// Flag to validate if the sequence is running.
    bool is_running_{false};


    /**
     * Check the sequence for syntactic consistency and throw an exception if an error is
     * detected. That means that one or all of the following conditions must be satisfied:
     *
     * -# each TRY step must have a corresponding CATCH and END step
     * -# each IF step must have m ELSEIF steps followed by n ELSE steps and one END step
     *    with m >= 0 and (n == 0 or n == 1).
     * -# each WHILE must have a matching END
     *
     * @param begin Iterator pointing to the first step to be checked
     * @param end   Iterator pointing past the last step to be checked
     * @exception Error is thrown if a syntax error is found.
     * @see check_syntax()
     */
    void check_syntax(ConstIterator begin, ConstIterator end) const;

    /**
     * Internal syntax check for if-elseif-else-clauses. Invoked by
     * check_syntax(ConstIterator, ConstIterator).
     *
     * @param begin Iterator pointing to the IF step; must be dereferenceable.
     * @param end   Iterator pointing past the last step to be checked
     * @returns an iterator pointing to the first step after the IF..(ELSEIF)..(ELSE)..END
     *          construct.
     * @exception Error is thrown if an ill-formed 'if-elseif-else' token is found.
     */
    ConstIterator check_syntax_for_if(ConstIterator begin, ConstIterator end) const;

    /**
     * Internal syntax check for try-catch-clauses. Invoked by
     * check_syntax(ConstIterator, ConstIterator).
     *
     * @param begin Iterator pointing to the TRY step; must be dereferenceable.
     * @param end   Iterator pointing past the last step to be checked
     * @returns an iterator pointing to the first step after the TRY..CATCH..END construct.
     * @exception Error is thrown if a syntax error is found.
     */
    ConstIterator check_syntax_for_try(ConstIterator begin, ConstIterator end) const;

    /**
     * Internal syntax check for while-clauses. Invoked by
     * check_syntax(ConstIterator, ConstIterator).
     *
     * @param begin Iterator pointing to the WHILE step; must be dereferenceable.
     * @param end   Iterator pointing past the last step to be checked
     * @returns an iterator pointing to the first step after the WHILE..END construct.
     * @exception Error is thrown if a syntax error is found.
     */
    ConstIterator check_syntax_for_while(ConstIterator begin, ConstIterator end) const;

    /**
     * Update the disabled flag of all steps to ensure that control structures are not
     * partially disabled.
     *
     * In particular, if the starting step of a control structure is disabled, all steps
     * contained within it as well as the final end are disabled. If the starting step of
     * a control structure is enabled, all associated control steps (else, elseif, catch,
     * end) are enabled as well.
     *
     * \pre
     * The steps must be correctly indented as per calling indent().
     */
    void enforce_consistency_of_disabled_flags() noexcept;

    /**
     * Make sure that all class invariants are upheld.
     *
     * This call updates the indentation and the "disabled" flags. It does not throw
     * exceptions except for, possibly, std::bad_alloc.
     */
    void enforce_invariants();

    /**
     * Execute an ELSE block.
     *
     * \param begin    Iterator to an ELSE step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param comm     Pointer to a communication channel; if null, messaging and
     *                 cross-thread interaction are disabled.
     *
     * \returns an iterator to the first step after the matching END step.
     */
    Iterator
    execute_else_block(Iterator begin, Iterator end, Context& context, CommChannel* comm);

    /**
     * Execute an IF or ELSEIF block.
     *
     * \param begin    Iterator to an IF or ELSEIF step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param comm     Pointer to a communication channel; if null, messaging and
     *                 cross-thread interaction are disabled.
     *
     * \returns an iterator to the step to be executed next: If the IF/ELSEIF evaluated as
     *          true, this is the first step after the matching END. Otherwise, it is the
     *          next step after skipping the current IF/ELSEIF block.
     */
    Iterator
    execute_if_or_elseif_block(Iterator begin, Iterator end, Context& context,
                               CommChannel* comm);

    /**
     * Execute a range of steps.
     *
     * \param step_begin Iterator to the first step that should be executed
     * \param step_end   Iterator past the last step that should be executed
     * \param context    Context for executing the steps
     * \param comm     Pointer to a communication channel; if null, messaging and
     *                 cross-thread interaction are disabled.
     * \exception Error is thrown if the execution fails at some point.
     */
    Iterator
    execute_sequence_impl(Iterator step_begin, Iterator step_end, Context& context,
                          CommChannel* comm);

    /**
     * Execute a TRY block.
     *
     * \param begin    Iterator to the TRY step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param comm     Pointer to a communication channel; if null, messaging and
     *                 cross-thread interaction are disabled.
     *
     * \returns an iterator to the first step after the matching END step.
     */
    Iterator
    execute_try_block(Iterator begin, Iterator end, Context& context, CommChannel* comm);

    /**
     * Execute a WHILE block.
     *
     * \param begin    Iterator to the WHILE step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param comm     Pointer to a communication channel; if null, messaging and
     *                 cross-thread interaction are disabled.
     *
     * \returns an iterator to the first step after the matching END step.
     */
    Iterator
    execute_while_block(Iterator begin, Iterator end, Context& context, CommChannel* comm);

    /**
     * Return an iterator past the END step that ends the block-with-continuation starting
     * at a given iterator.
     *
     * \code
     * WHILE    <- block_start
     *   ACTION
     * END
     * ACTION   <- find_end_of_continuation(block_start)
     * \endcode
     *
     * \returns an iterator past the matching END step or steps_.end() if there is no
     *          matching END step.
     */
    Iterator find_end_of_continuation(Iterator block_start);
    ConstIterator find_end_of_continuation(ConstIterator block_start) const;

    /**
     * Assign indentation levels to all steps according to their logical nesting.
     *
     * If errors in the logical nesting are found, an approximate indentation is assigned
     * and the member string indentation_error_ is filled with an error message. If the
     * nesting is correct and complete, indentation_error_ is set to an empty string.
     *
     * This function does not throw exceptions except for, possibly, std::bad_alloc.
     */
    void indent();

    /// Throw an Error if no further steps can be inserted into the sequence.
    void throw_if_full() const;

    /// When the sequence is executed it rejects with an Error exception.
    void throw_if_running() const;

    /**
     * Throw a syntax error for the specified step.
     * The error message reports the step number.
     */
    void throw_syntax_error_for_step(ConstIterator it, gul14::string_view msg) const;
};

} // namespace task

#endif
