/**
 * \file   Sequence.h
 * \author Marcus Walla
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

#ifndef TASKOMAT_SEQUENCE_H_
#define TASKOMAT_SEQUENCE_H_

#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>
#include <gul14/gul.h>
#include "taskomat/Context.h"
#include "taskomat/Error.h"
#include "taskomat/Message.h"
#include "taskomat/Step.h"

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
 * -# insert(): insert a Step or a range of steps
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
 */
class Sequence
{
public:
    /// Abbreviation for steps.
    using Steps = std::vector<Step>;
    using SizeType = std::uint16_t;
    using size_type = SizeType;
    using Iterator = Steps::iterator;
    using ReverseIterator = Steps::reverse_iterator;
    using ConstIterator = Steps::const_iterator;
    using ConstReverseIterator = Steps::const_reverse_iterator;

    static constexpr std::size_t max_label_length = 128;

    /**
     * Construct a Sequence with a descriptive name.
     * The label should describe the function of the sequence clearly and concisely.
     *
     * \param label [IN] descriptive and clear label.
     *
     * \exception Error is thrown if the label is empty or if its length exceeds
     *            max_label_length characters.
     */
    explicit Sequence(gul14::string_view label = "anonymous");

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

    /// Determine whether the sequence contains no steps.
    bool empty() const noexcept { return steps_.empty(); }

    /**
     * Access the step at a given index.
     *
     * The index operator can only be used for read access to the sequence steps. This is
     * because Sequence has to maintain invariants such as the correct indentation
     * whenever steps are modified.
     */
    const Step& operator[](SizeType idx) const noexcept { return steps_[idx]; }

    /**
     * Return a constant iterator to the first Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstIterator begin() const noexcept { return steps_.cbegin(); }
    ConstIterator cbegin() const noexcept { return steps_.cbegin(); }

    /**
     * Return a constant reverse iterator to the last Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstReverseIterator rbegin() const noexcept { return steps_.crbegin(); }
    ConstReverseIterator crbegin() const noexcept { return steps_.crbegin(); }

    /**
     * Return a constant iterator pointing past the last Step in the container.
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstIterator end() const noexcept { return steps_.cend(); }
    ConstIterator cend() const noexcept { return steps_.cend(); }

    /**
     * Return a constant reverse iterator pointing before the first Step in the container.
     * element of the
     *
     * Non-const iterators are not available because Sequence has to maintain invariants
     * such as the correct indentation whenever steps are modified.
     */
    ConstReverseIterator rend() const noexcept { return steps_.crend(); }
    ConstReverseIterator crend() const noexcept { return steps_.crend(); }

    /// Return the number of steps contained in this sequence.
    SizeType size() const noexcept { return static_cast<SizeType>(steps_.size()); }

    /**
     * Add \a Step to the sequence.
     *
     * @param step [IN] \a Step
     * @deprecated Replace by push_back(const Step&). Can be removed in future releases.
     */
    [[deprecated("No longer supported")]]
    void add_step(const Step& step) { push_back(step); }

    /**
     * Add \a Step to the sequence.
     *
     * @param step [MOVE] \a Step
     * @deprecated Replace by push_back(Step&&). Can be removed in future releases.
     */
    [[deprecated("No longer supported")]]
    void add_step(Step&& step) { push_back(std::move(step)); }

    /**
     * Attach \a Step reference to the end of the sequence.
     *
     * @param step [IN] \a Step
     */
    void push_back(const Step& step) { steps_.push_back(step); indent(); }

    /**
     * Attach \a Step rvalue reference to the end of the sequence.
     *
     * @param step [MOVE] \a Step
     */
    void push_back(Step&& step) { steps_.push_back(step); indent(); }

    /**
     * Remove the last element from the sequence.
     *
     * Calling pop_back() on an empty Sequence returns silently. Iterators and references to the
     * last element as well as the end() iterator are invalidated.
     */
    void pop_back() { if (not steps_.empty()) steps_.pop_back(); indent(); }

    /**
     * Insert the given \a Step before of the constant iterator into Sequence.
     *
     * When the size increase the capacity a reallocation is performed that invalidates
     * all iterators. One can check with has_valid_iterators().
     *
     * @param iter constant \a Step iterator in the Sequence
     * @param step the added \a Step
     * @return inserted \a Step
     */
    ConstIterator insert(ConstIterator iter, const Step& step)
    {
        auto return_iter = steps_.insert(iter, step);
        indent();
        return return_iter;
    }

    /**
     * Insert the given \a Step rvalue reference before of the constant iterator into
     * Sequence.
     *
     * When the size increase the capacity a reallocation is performed that invalidates
     * all iterators. One can check with has_valid_iterators().
     *
     * @param iter constant \a Step iterator in the Sequence
     * @param step the added \a Step
     * @return inserted \a Step
     */
    ConstIterator insert(ConstIterator iter, Step&& step)
    {
        auto return_iter = steps_.insert(iter, std::move(step));
        indent();
        return return_iter;
    }

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
    ConstIterator erase(ConstIterator iter)
    {
        auto return_iter = steps_.erase(iter);
        indent();
        return return_iter;
    }

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
    ConstIterator erase(ConstIterator first, ConstIterator last)
    {
        auto return_iter = steps_.erase(first, last);
        indent();
        return return_iter;
    }

    /**
     * Modify the \a Step at the given position.
     *
     * @param iter position to assign the \a Step
     * @param step \a Step
     */
    void assign(ConstIterator iter, const Step& step)
    {
        auto it = steps_.begin() + (iter - steps_.cbegin());
        *it = step;
        indent();
    }

    /**
     * Modify the \a Step at the given position.
     *
     * @param iter position to assign the \a Step
     * @param step refernce value \a Step
     */
    void assign(ConstIterator iter, Step&& step)
    {
        auto it = steps_.begin() + (iter - steps_.cbegin());
        *it = std::move(step);
        indent();
    }

    /**
     * Execute the sequence within a given context.
     *
     * This function first performs a syntax check and throws an Error exception if it
     * fails. Then, it executes the sequence step by step, following the control flow
     * given by steps such as WHILE, IF, TRY, and so on. It returns when the sequence is
     * finished or has stopped with an error.
     *
     * \param context A context for storing variables that can be exchanged between
     *                different steps. The context may also contain a LUA init function
     *                that is run before each step.
     * \param queue   Pointer to a message queue. If this is a null pointer, no messages
     *                are sent. Otherwise, messages for starting/stopping steps and the
     *                sequence itself are sent.
     * \exception Error is thrown if the script cannot be executed due to a syntax error
     *            or if it raises an error during execution.
     */
    void execute(Context& context, MessageQueue* queue);

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
     * \param it  An iterator to the Step that should be modified. The step must be part
     *            of this sequence.
     * \param modification_fct  A function or function object with the signature
     *            `void fct(Step&)` that applies the desired modifications on a Step.
     *            The step reference becomes invalid after the call.
     *
     * \exception
     * If an exception is thrown by the modification function, the step may only be
     * partially modified, but the invariants of the sequence are maintained (basic
     * exception guarantee).
     */
    template <typename Closure>
    void modify(ConstIterator it, Closure modification_fct)
    {
        // Reindent at the end of the function, even if an exception is thrown
        auto indent_if_necessary = gul14::finally(
            [this,
             it,
             old_indentation_level = it->get_indentation_level(),
             old_type = it->get_type()]()
            {
                if (it->get_indentation_level() != old_indentation_level ||
                    it->get_type() != old_type)
                {
                    indent();
                }
            });

        auto mutable_it = steps_.begin() + (it - steps_.cbegin());

        modification_fct(*mutable_it);
    }

private:
    /// Empty if indentation is correct and complete, error message otherwise
    std::string indentation_error_;

    std::string label_;
    Steps steps_;

    /// Check that the given description is valid. If not then throws a task::Error.
    void check_label(gul14::string_view label);

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
     * @exception throws an \a Error exception if an ill-formed token is found.
     * @see check_syntax()
     */
    void check_syntax(ConstIterator begin, ConstIterator end) const;

    /**
     * Internal syntax check for while-clauses. Invoked by
     * \a check_syntax(const int, SizeType).
     *
     * @param begin Iterator pointing to the WHILE step; must be dereferenceable.
     * @param end   Iterator pointing past the last step to be checked
     * @returns an iterator pointing to the first step after the WHILE..END construct.
     * @exception throws an \a Error exception if an ill-formed 'while' token is found.
     */
    ConstIterator check_syntax_for_while(ConstIterator begin, ConstIterator end) const;

    /**
     * Internal syntax check for try-catch-clauses. Invoked by
     * \a check_syntax(const int, SizeType).
     *
     * @param begin Iterator pointing to the TRY step; must be dereferenceable.
     * @param end   Iterator pointing past the last step to be checked
     * @returns an iterator pointing to the first step after the TRY..CATCH..END construct.
     * @exception throws an \a Error exception if an ill-formed 'try' token is found.
     */
    ConstIterator check_syntax_for_try(ConstIterator begin, ConstIterator end) const;

    /**
     * Internal syntax check for if-elseif-else-clauses. Invoked by
     * \a check_syntax(const int, SizeType).
     *
     * @param begin Iterator pointing to the IF step; must be dereferenceable.
     * @param end   Iterator pointing past the last step to be checked
     * @returns an iterator pointing to the first step after the IF..(ELSEIF)..(ELSE)..END
     *          construct.
     * @exception Error is thrown if an ill-formed 'if-elseif-else' token is found.
     */
    ConstIterator check_syntax_for_if(ConstIterator begin, ConstIterator end) const;

    /**
     * Execute an ELSE block.
     *
     * \param begin    Iterator to an ELSE step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param queue    Pointer to a thread-safe message queue; if null, messaging is
     *                 disabled.
     *
     * \returns an iterator to the first step after the matching END step.
     */
    Iterator
    execute_else_block(Iterator begin, Iterator end, Context& context,
                       MessageQueue* queue);

    /**
     * Execute an IF or ELSEIF block.
     *
     * \param begin    Iterator to an IF or ELSEIF step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param queue    Pointer to a thread-safe message queue; if null, messaging is
     *                 disabled.
     *
     * \returns an iterator to the step to be executed next: If the IF/ELSEIF evaluated as
     *          true, this is the first step after the matching END. Otherwise, it is the
     *          next step after skipping the current IF/ELSEIF block.
     */
    Iterator
    execute_if_or_elseif_block(Iterator begin, Iterator end, Context& context,
                               MessageQueue* queue);

    /**
     * Execute a range of steps.
     *
     * \param step_begin Iterator to the first step that should be executed
     * \param step_end   Iterator past the last step that should be executed
     * \param context    Context for executing the steps
     * \exception Error is thrown if the execution fails at some point.
     */
    Iterator
    execute_sequence_impl(Iterator step_begin, Iterator step_end, Context& context,
                          MessageQueue* queue);

    /**
     * Execute a TRY block.
     *
     * \param begin    Iterator to the TRY step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param queue    Pointer to a thread-safe message queue; if null, messaging is
     *                 disabled.
     *
     * \returns an iterator to the first step after the matching END step.
     */
    Iterator
    execute_try_block(Iterator begin, Iterator end, Context& context, MessageQueue* queue);

    /**
     * Execute a WHILE block.
     *
     * \param begin    Iterator to the WHILE step
     * \param end      Iterator past the last step to be scanned for matching indentation
     *                 (may simply be end())
     * \param context  Execution context
     * \param queue    Pointer to a thread-safe message queue; if null, messaging is
     *                 disabled.
     *
     * \returns an iterator to the first step after the matching END step.
     */
    Iterator
    execute_while_block(Iterator begin, Iterator end, Context& context,
                        MessageQueue* queue);

    /**
     * Assign indentation levels to all steps according to their logical nesting.
     *
     * If errors in the logical nesting are found, an approximate indentation is assigned
     * and the member string indentation_error_ is filled with an error message. If the
     * nesting is correct and complete, indentation_error_ is set to an empty string.
     */
    void indent() noexcept;

    /**
     * Throw a syntax error for the specified step.
     * The error message reports the step number.
     */
    void throw_syntax_error_for_step(ConstIterator it, gul14::string_view msg) const;
};

} // namespace task

#endif
