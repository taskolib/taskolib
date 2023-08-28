/**
 * \file   hash_string.h
 * \author Richard Spencer
 * \date   Created on January 15, 2022
 * \brief  Allow string literals as case labels, -std=c++17.
 *
 * \copyright Copyright 2020-2022 Richard Spencer
 *
 * Version 1.4: 2022/01/15, MIT license, (c) 2020-22 Richard Spencer
 *
 * Adaptations for taskolib: L. Fröhlich, U. Jastrow, M. Walla, 2022.
 *
 * MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// SPDX-License-Identifier: MIT

#ifndef TASKOLIB_CASE_STRING_H_
#define TASKOLIB_CASE_STRING_H_

#include <gul14/string_view.h>

namespace task {

/// Adapt switch statement with stringify items:
/// https://learnmoderncpp.com/2020/06/01/strings-as-switch-case-labels/
/// Version 1.4: 2022/01/15, MIT license, (c) 2020-22 Richard Spencer
inline constexpr unsigned long hash_djb2a(gul14::string_view sv) {
    unsigned long hash{ 5381 };
    for (unsigned char c : sv) {
        hash = ((hash << 5) + hash) ^ c;
    }
    return hash;
}

/// Adapt switch statement with stringify items:
/// https://learnmoderncpp.com/2020/06/01/strings-as-switch-case-labels/
/// Version 1.4: 2022/01/15, MIT license, (c) 2020-22 Richard Spencer
inline constexpr unsigned long operator"" _sh(const char *str, std::size_t len) {
    return hash_djb2a(gul14::string_view{ str, len });
}

} // namespace task

#endif
