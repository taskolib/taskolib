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

#ifndef AVTOMAT_SEQEUNCE_H_
#define AVTOMAT_SEQEUNCE_H_

#include <string>
#include <vector>
#include <gul14/string_view.h>
#include <gul14/cat.h>
#include "avtomat/Error.h"
#include "avtomat/Context.h"
#include "avtomat/Step.h"

namespace avto {

/**
 * \brief A sequence of \a Step 's to be executed under a given \a Context .
 * 
 * On executing a validation is performed due to check if the steps are consistent. When a fault
 * is detected an \a Error is thrown including a pricese error message about what fails. 
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
     * \param name [IN] descriptive and clear name.
     */
    explicit Sequence( const std::string& name ) noexcept: name_{ name } { check_name( name_ ); }
    explicit Sequence( std::string&& name ) noexcept: name_{ std::move( name ) } { check_name( name_ ); }

    /**
     * @brief Get the descriptive name.
     * 
     * @return std::string [OUT] descriptive name
     */
    inline std::string get_name() const noexcept 
    {
        return this->name_;
    }

    /**
     * @brief Add \a Step to the sequence.
     * 
     * @param step [IN/OUT/MOVE] Step
     */
    inline void add_step( const Step& step ) noexcept { this->steps.push_back( step ); }
    inline void add_step( Step&& step ) noexcept { this->steps.push_back( std::move( step ) ); }
 
    /**
     * @brief Validates if the \a Step 's are correctly enclosed in a proper way.
     * 
     * It is done by validating the step types where each of the following condition:
     * 
     * -# each type \a avto::Step::Type::type_try must have the corresponding \a avto::Step::Type::type_catch
     * -# each type \a avto::Step::Type::type_if , \a avto::Step::Type::type_while , and 
     *  \a  \a avto::Step::Type::type_try must have the corresponding \a  \a avto::Step::Type::type_end 
     * 
     * If one of those is false an avto::Error exception is thrown.
     * 
     * @param return always true - only for the sake of the test cases.
     */
    bool check_correctness_of_steps();

    /**
     * @brief Execute the sequence unter context \a Context with the required variables.
     * 
     * The variables are copied for step type \a avto::Step::Type::type_action from step to 
     * step with their intermediate changed values from the previous step.
     * 
     * On error the method will throw an \a avto::Error including a precise description
     * where the fault occured. You can also examine the variables in \a avto::Context .
     * 
     * @param context [IN/OUT] context with starting variable declaration.
     */
    void execute( Context& context );

private:
    enum E_IF { IF_NONE = 0, HAS_IF, HAS_ELSE_IF, HAS_ELSE };
    enum E_WHILE { WHILE_NONE = 0, HAS_WHILE };
    enum E_TRY { TRY_NONE = 0, HAS_TRY, HAS_CATCH };

    std::string name_;
    std::vector<Step> steps;

    /// Check that the given description is valid. If not then throw an avto::Error.
    void check_name( gul14::string_view name )
    {
        if ( name.empty() )
            throw Error( "Descriptive string may not be empty" );

        if ( name.size() > 64 )
            throw Error( gul14::cat( "Descriptive string \"", name, "\" is too long (>64 characters)" ) );
    }
};

}

#endif