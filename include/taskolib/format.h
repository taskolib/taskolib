/**
 * \file   format.h
 * \author Fini Jastrow
 * \date   Created on June 22, 2022
 * \brief  Helper for fmt{} library to print our types.
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

#ifndef TASKOLIB_FORMAT_H_
#define TASKOLIB_FORMAT_H_

#include <fmt/format.h>
#include <sstream>

#include "taskolib/Message.h"
#include "taskolib/time_types.h"
#include "taskolib/Timeout.h"

using namespace std::literals;

template<> struct fmt::formatter<task::Message> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(task::Message const& val, FormatContext& ctx) -> decltype(ctx.out()) {
        std::stringstream ss{ };
        ss << val;
        return format_to(ctx.out(), "{}", ss.str());
    }
};

template<> struct fmt::formatter<task::Timeout> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(task::Timeout const& val, FormatContext& ctx) -> decltype(ctx.out()) {
        std::stringstream ss{ };
        ss << val;
        return format_to(ctx.out(), "{}", ss.str());
    }
};

template<> struct fmt::formatter<task::TimePoint> {
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        return ctx.begin();
    }
    template <typename FormatContext>
    auto format(task::TimePoint const& val, FormatContext& ctx) -> decltype(ctx.out()) {
        return format_to(ctx.out(), "{}", task::to_string(val));
    }
};

#endif
