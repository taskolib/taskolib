/**
 * \file   Step.h
 * \author Lars Froehlich
 * \date   Created on November 26, 2021
 * \brief  Declaration of the Step class.
 *
 * \copyright Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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

#ifndef AVTOMAT_STEP_H_
#define AVTOMAT_STEP_H_

#include <string>

namespace avto {


/**
 * A step holds a script and can execute it.
 * Steps are the building blocks for sequences.
 */
class Step
{
public:
    /**
     * Return the script.
     *
     * \note
     * This function returns a reference to an internal member variable, so be aware of
     * lifetime implications:
     * \code
     * Step my_step;
     *
     * // Safe, string is copied
     * std::string str = my_step.get_script();
     *
     * // Fast, but str_ref will dangle if my_step goes out of scope
     * const std::string& str_ref = my_step.get_script();
     * \endcode
     */
    const std::string& get_script() const { return script_; }

    /**
     * Set the script that should be executed when this step is run.
     * At this point, no syntax or semantics check takes place.
     */
    void set_script(const std::string& script) { script_ = script; }

private:
    std::string script_;
};


} // namespace avto

#endif
