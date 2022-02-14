/**
 * \file   Sequencer.h
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
#include "taskomat/Context.h"
#include "taskomat/Step.h"

namespace task {

/**
 * \brief A sequence of \a Step 's to be executed under a given \a Context .
 *
 * On executing a validation is performed due to check if the steps are consistent. When
 * a fault is detected an \a Error is thrown including a pricese error message about what
 * fails.
 */
class Sequence
{
    public:
    /**
     * \brief Construct a Sequence with descriptive name.
     *
     * By declaring a Sequence with an empty name or exceeding a length of 64 characters
     * will throw an \a Error exception.
     *
     * There is now presure to phrase an unqmbiguous description but it would be good for
     * other colleagues to fetch the significance.
     *
     * \param label [IN] descriptive and clear name.
     */
    explicit Sequence( const std::string& label = "[anonymous]" ): label_{ 
        label } { check_label( label_ ); }
    explicit Sequence( std::string&& label = "[anonymous]" ): label_{ 
        std::move( label ) } { check_label( label_ ); }

    /**
     * @brief Get the descriptive name.
     *
     * @return std::string [OUT] descriptive name
     */
    std::string get_label() const
    {
        return this->label_;
    }

    /**
     * @brief Add \a Step to the sequence.
     *
     * @param step [IN/MOVE] Step
     */
    void add_step( const Step& step ) noexcept { this->steps_.push_back( step ); }
    void add_step( Step&& step ) noexcept { this->steps_.push_back( std::move( step ) ); }

    /**
     * @brief Validates if the \a Step 's are correctly enclosed in a proper way.
     *
     * It is done by validating the step types where each of the following condition:
     *
     * -# each type \a task::Step::Type::type_try must have the corresponding
     *    \a task::Step::Type::type_catch
     * -# each type \a task::Step::Type::type_if , \a task::Step::Type::type_while , and
     *  \a task::Step::Type::type_try must have the corresponding
     *  \a task::Step::Type::type_end
     * -# must be filled...
     *
     * If one of those is false an task::Error exception is thrown.
     */
    void check_correctness_of_steps();

    /**
     * @brief Execute the sequence under context \a Context with required variables.
     *
     * The variables are copied for step type \a task::Step::Type::type_action from
     * step to step with their intermediate changed values from the previous step.
     *
     * On error the method will throw an \a task::Error including a precise description
     * where the fault occured. You can also examine the variables in \a task::Context .
     *
     * @param context [IN/OUT] context with starting variable definition.
     */
    void execute( Context& context );

private:
    enum E_IF { NO_IF = 0, IF, ELSE_IF, ELSE };
    enum E_WHILE { NO_WHILE = 0, WHILE };
    enum E_TRY { NO_TRY = 0, TRY, CATCH };
    enum E_ACTION { NO_ACTION = 0, ACTION };
    enum E_END { NO_END, END };

    std::string label_;
    std::vector<Step> steps_;

    /// Check that the given description is valid. If not then throw an task::Error.
    void check_label( gul14::string_view label )
    {
        if ( label.empty() )
            throw Error( "Descriptive string may not be empty" );

        if ( label.size() > 64 )
            throw Error( gul14::cat( "Descriptive string \"", label, "\" is too long (>64"
            " characters)" ) );
    }
};

} // namespace task

#endif
