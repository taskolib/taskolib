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
 * A sequence of \a Step 's to be executed under a given \a Context .
 *
 * On executing a validation is performed due to check if the steps are consistent. When
 * a fault is detected an \a Error is thrown including a precise error message about what
 * fails.
 */
class Sequence
{
public:
    /// Abbraviation for steps.
    using Steps = std::vector<Step>;
    using SizeType = std::uint16_t;
    using size_type = SizeType;

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
     * Add \a Step to the sequence.
     *
     * @param step [IN] Step
     */
    void add_step( const Step& step ) { steps_.push_back(step); indent(); }
    /**
     * Add \a Step to the sequence.
     *
     * @param step [MOVE] Step
     */
    void add_step( Step&& step ) { steps_.push_back(std::move(step)); indent(); }

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
    const Steps::iterator begin() noexcept { return steps_.begin(); }
    /// Return constant Steps iterator to the first element of the container.
    const Steps::const_iterator begin() const noexcept { return steps_.begin(); }

    /// Return Steps iterator to the element following the last element of the container.
    const Steps::iterator end() noexcept { return steps_.end(); }
    /// Return constant Steps iterator to the element following the last element of the
    /// container.
    const Steps::const_iterator end() const noexcept { return steps_.end(); }

    /// Return the number of steps contained in this sequence.
    SizeType size() const noexcept { return static_cast<SizeType>(steps_.size()); }

private:

    /// Empty if indentation is correct and complete, error message otherwise
    std::string indentation_error_;

    std::string label_;
    Steps steps_;

    /// Check that the given description is valid. If not then throw a task::Error.
    void check_label(gul14::string_view label);

    /**
     * Check the sequence for syntactic consistency and throw an exception if an error is
     * detected. That means that one or all of the following conditions must be satisfied:
     * 
     * -# each token \a Step::type_try must have the corresponding
     *    \a Step::type_catch and \a Step::type_end
     * -# each token \a Step::type_if must have n-times \a Step::type_elseif and/or
     *  \a Step::type_else with a tailing \a Step::type_end, n >= 0.
     * -# each token \a Step::while must have the corresponding \a Step::type_end
     *
     * As a body of each surrounding token it must have at least one \a Step::type_action 
     * as Lua scriptless.
     * 
     * @param level nested indention level to check. Base level is 0.
     * @param idx index of step in sequence.
     * @exception throws an \a Error exception if an ill-formed token is found.
     * @see #check_syntax()
     */
    void check_syntax(const short level, SizeType idx) const;

    /**
     * Internal syntax check for while-clauses. Invoked by 
     * \a check_syntax(const int, SizeType).
     * 
     * @param level nested indention level to check.
     * @param idx index of step in sequence.
     * @return new evaluated index for next token.
     * @exception throws an \a Error exception if an ill-formed 'while' token is found.
     */
    SizeType syntax_checker_for_while(const short level, SizeType idx) const;

    /**
     * Internal syntax check for try-catch-clauses. Invoked by 
     * \a check_syntax(const int, SizeType).
     * 
     * @param level nested indention level to check.
     * @param idx index of step in sequence.
     * @return new evaluated index for next token.
     * @exception throws an \a Error exception if an ill-formed 'try' token is found.
     */
    SizeType syntax_checker_for_try(const short level, SizeType idx) const;

    /**
     * Internal syntax check for if-elseif-else-clauses. Invoked by 
     * \a check_syntax(const int, SizeType).
     * 
     * @param level nested indention level to check.
     * @param idx index of step in sequence.
     * @return new evaluated index for next token.
     * @exception throws an \a Error exception if an ill-formed 'if-elseif-else' token
     * is found.
     */
    SizeType syntax_checker_for_if(const short level, SizeType idx) const;

    /**
     * Assign indentation levels to all steps according to their logical nesting.
     *
     * If errors in the logical nesting are found, an approximate indentation is assigned
     * and the member string indentation_error_ is filled with an error message. If the
     * nesting is correct and complete, indentation_error_ is set to an empty string.
     */
    void indent() noexcept;
};

} // namespace task

#endif
