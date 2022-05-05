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

#include <string>
#include <vector>
#include <gul14/string_view.h>
#include <gul14/cat.h>
#include "taskomat/Error.h"
#include "taskomat/Step.h"

namespace task {

/**
 * \section Main Sequencer class
 *
 * A sequence of \a Step 's to be executed under a given \a Context .
 *
 * On executing a validation is performed due to check if the steps are consistent. When
 * a fault is detected an \a Error is thrown including a precise error message about what
 * fails.
 *
 * To modify the sequence the following member functions are implemented:
 *
 * -# push_back(): add new \a Step at the end
 * -# pop_back(): remove last \a Step from the end
 * -# insert(): insert \a Step or range of \a Step 's either with position or iterator.
 * -# assign(): assign a new \a Step to an existing element.
 * -# erase(): remove \a Step or range of \a Step 's either with position or iterator.
 *
 * After modifing the sequence all before retrieved iterators are invalidated and further
 * usage will result with an undefined behavior.
 *
 * Since the reverse iterator are only used to iterate through the sequence one will
 * not find any member function for manipulation or modification.
 *
 * \note To use one of the member functions for modification with index one can use the
 * following workaround:
 *
 * \code {.cpp}
 * Sequence seq;
 * seq.push_back(Step{Step::type_try});
 * seq.push_back(Step{Step::type_action});
 * seq.push_back(Step{Step::type_catch});
 * seq.push_back(Step{Step::type_action});
 * seq.push_back(Step{Step::type_end});
 * // insert at index 1 an action step
 * seq.insert(seq.begin()+1, Step{Step::type_action});
 * \endcode
 */
class Sequence
{
public:
    /// Abbraviation for steps.
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
    explicit Sequence(gul14::string_view label = "[anonymous]");

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
     * The index operator can only be used for read access to the sequence steps.
     */
    const Step& operator[](SizeType idx) const { return steps_[idx]; }

    /// Return Steps iterator to the first element of the container.
    const Iterator begin() noexcept { return steps_.begin(); }
    /// Return constant Steps iterator to the first element of the container.
    const ConstIterator cbegin() const noexcept { return steps_.cbegin(); }

    /// Return reverse Steps iterator to the first element of the container.
    const ReverseIterator rbegin() noexcept { return steps_.rbegin(); }
    /// Return constant reverse Steps iterator to the first element of the container.
    const ConstReverseIterator crbegin() const noexcept { return steps_.crbegin(); }

    /// Return Steps iterator to the element following the last element of the container.
    const Iterator end() noexcept { return steps_.end(); }
    /// Return constant Steps iterator to the element following the last element of the
    /// container.
    const ConstIterator cend() const noexcept { return steps_.cend(); }

    /// Return reverse Steps iterator to the element following the last element of the
    /// container.
    const ReverseIterator rend() noexcept { return steps_.rend(); }
    /// Return constant reverse Steps iterator to the element following the last element
    /// of the container.
    const ConstReverseIterator crend() const noexcept { return steps_.crend(); }

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
    Sequence::ConstIterator check_syntax_for_while(Sequence::ConstIterator begin,
        Sequence::ConstIterator end) const;

    /**
     * Internal syntax check for try-catch-clauses. Invoked by
     * \a check_syntax(const int, SizeType).
     *
     * @param begin Iterator pointing to the TRY step; must be dereferenceable.
     * @param end   Iterator pointing past the last step to be checked
     * @returns an iterator pointing to the first step after the TRY..CATCH..END construct.
     * @exception throws an \a Error exception if an ill-formed 'try' token is found.
     */
    Sequence::ConstIterator check_syntax_for_try(Sequence::ConstIterator begin,
        Sequence::ConstIterator end) const;

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
    Sequence::ConstIterator check_syntax_for_if(Sequence::ConstIterator begin,
        Sequence::ConstIterator end) const;

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


namespace detail {

Sequence::ConstIterator find_end_of_indented_block(Sequence::ConstIterator begin,
    Sequence::ConstIterator end, short min_indentation_level);

} // namespace task::detail


} // namespace task

#endif
