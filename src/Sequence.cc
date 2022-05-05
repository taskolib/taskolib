/**
 * \file   Sequence.cc
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
#include "taskomat/execute_step.h"
#include "taskomat/Sequence.h"

using gul14::cat;

namespace task
{

const char Sequence::head[] = "[syntax check] ";

Sequence::Sequence(gul14::string_view label)
{
    check_label(label);
    label_ = std::string{ label };
}

void Sequence::check_syntax() const
{
    if (not indentation_error_.empty())
        throw Error(indentation_error_);

    if (not steps_.empty())
        check_syntax(0, 0);
}

void Sequence::check_label(gul14::string_view label)
{
    if (label.empty())
        throw Error("Sequence label may not be empty");

    if (label.size() > max_label_length)
    {
        throw Error(cat("Label \"", label, "\" is too long (>", max_label_length,
                        " characters)"));
    }
}

void Sequence::indent() noexcept
{
    short level = 0;

    indentation_error_.clear();

    for (Step& step : steps_)
    {
        short step_level = -1;

        switch (step.get_type())
        {
            case Step::type_action:
                step_level = level;
                break;
            case Step::type_if:
            case Step::type_try:
            case Step::type_while:
                step_level = level;
                ++level;
                break;
            case Step::type_catch:
            case Step::type_else:
            case Step::type_elseif:
                step_level = level - 1;
                break;
            case Step::type_end:
                step_level = level - 1;
                --level;
                break;
        };

        if (step_level < 0)
        {
            step_level = 0;

            if (indentation_error_.empty())
                indentation_error_ = "Steps are not nested correctly";
        }

        step.set_indentation_level(step_level);// cannot throw because we check step_level

        if (level < 0)
        {
            level = 0;
            if (indentation_error_.empty())
            {
                indentation_error_ = "Steps are not nested correctly (every END must "
                    "correspond to one IF, TRY, or WHILE)";
            }
        }
        else if (level > Step::max_indentation_level)
        {
            level = Step::max_indentation_level;
            if (indentation_error_.empty())
            {
                indentation_error_ = cat("Steps are nested too deeply (max. level: ",
                                         Step::max_indentation_level, ')');
            }
        }
    }

    if (level != 0)
    {
        if (indentation_error_.empty())
        {
            indentation_error_ = "Steps are not nested correctly (there must be one END "
                "for each IF, TRY, WHILE)";
        }
    }
}

void Sequence::check_syntax(short level, Sequence::SizeType idx) const
{
    do
    {
        const short step_indention_level = steps_[idx].get_indentation_level();

        if (step_indention_level > level)
            check_syntax(step_indention_level, idx);
        else if (step_indention_level == level)
        {
            switch(steps_[idx].get_type())
            {
                case Step::type_while:
                    idx = check_syntax_for_while(step_indention_level, idx);
                    break;
                case Step::type_try:
                    idx = check_syntax_for_try(step_indention_level, idx);
                    break;
                case Step::type_if:
                    idx = check_syntax_for_if(step_indention_level, idx);
                    break;

                default:
                    break;
            }
        }
    } while(++idx < steps_.size());
}

Sequence::SizeType Sequence::check_syntax_for_while(const short level,
    Sequence::SizeType idx) const
{
    const Sequence::SizeType while_idx = idx;

    while(++idx < steps_.size())
    {
        if (steps_[idx].get_indentation_level() == level)
        {
            switch(steps_[idx].get_type())
            {
                case Step::type_end:
                    if (while_idx + 1 < idx)
                        return level == 0 ? while_idx : idx;
                    // fallthrough

                default:
                    throw Error(cat(head, "ill-formed while-clause: missing action or "
                    "control-flow token before 'end'. indent=", level, ", index=", idx,
                    ", previous 'while' indication=", while_idx));
            }
        }
    }

    throw Error(cat(head, "ill-formed while-clause: missing 'end' token. indent=", level,
    ", previous 'while' indication=", while_idx));
}

Sequence::SizeType Sequence::check_syntax_for_try(const short level,
    Sequence::SizeType idx) const
{
    const Sequence::SizeType try_idx = idx;
    Sequence::SizeType counter = idx;
    while(++idx < steps_.size())
    {
        if (steps_[idx].get_indentation_level() == level)
        {
            switch(steps_[idx].get_type())
            {
                case Step::type_catch:
                    if (counter + 1 >= idx)
                    {
                        throw Error(cat(head, "ill-formed try-clause: missing action or "
                        "control-flow token before 'catch'. indent=", level, ", index=",
                        idx, ", previous 'try' indication=", try_idx));
                    }
                    counter = idx;
                    break;

                case Step::type_end:
                    if (counter + 1 < idx)
                        return level == 0 ? try_idx : idx;
                    // fallthrough

                default:
                    throw Error(cat(head, "ill-formed try-clause: missing action or "
                    "control-flow token before 'end'. indent=", level, ", index=", idx,
                    ", previous 'try' indication=", try_idx));
            }
        }
    }

    throw Error(cat(head, "ill-formed try-clause: missing 'end' token. indent=", level,
    ", previous 'try' indication=", try_idx));
}

Sequence::SizeType Sequence::check_syntax_for_if(const short level,
    Sequence::SizeType idx) const
{
    const SizeType if_idx = idx;
    SizeType counter = idx;
    bool find_else = false;
    while(++idx < steps_.size())
    {
        if (steps_[idx].get_indentation_level() == level)
        {
            switch(steps_[idx].get_type())
            {
                case Step::type_elseif:
                    if (counter + 1 >= idx)
                    {
                        throw Error(cat(head, "ill-formed if-clause: missing action or "
                        "control-flow token. indent=", level, ", index=", idx,
                        ", previous 'if' indication=", if_idx));
                    }
                    else if (find_else)
                    {
                        throw Error(cat(head, "ill-formed if-clause: 'else' before 'else "
                        "if' token. indent=", level, ", index=", idx, ", previous 'if' "
                        "indication=", if_idx));
                    }
                    counter = idx;
                    break;

                case Step::type_else:
                    if (counter + 1 >= idx)
                    {
                        throw Error(cat(head, "ill-formed if-clause: missing action or "
                        "control-flow token. indent=", level, ", index=", idx,
                        ", previous 'if' indication=", if_idx));
                    }
                    counter = idx;
                    find_else = true;
                    break;

                case Step::type_end:
                    if (counter + 1 < idx)
                        return level == 0 ? if_idx : idx;
                    // fallthrough

                default:
                    throw Error(cat(head, "ill-formed if-clause: missing action or "
                    "control-flow token before 'end'. indent=", level, ", index=", idx,
                    ", previous 'if' indication=", if_idx));
            }
        }
    }

    throw Error(cat(head, "ill-formed if-clause: missing 'end' token. indent=", level,
    ", previous 'if' indication=", if_idx));
}

} // namespace task
