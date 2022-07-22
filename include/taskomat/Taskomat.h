/**
 * \file   Taskomat.h
 * \author Marcus Walla
 * \date   Created on July 22, 2022
 * \brief  Manage and control sequences.
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

#ifndef TASKOMAT_TASKOMAT_OVERVIEW_H_
#define TASKOMAT_TASKOMAT_OVERVIEW_H_

#include <gul14/string_view.h>

namespace task {

/**
 * A class to have a birds eye view on the underlying serialized sequences in the file
 * system. It allows to manage and control the sequences.
 *
 * [NOTE: NEEDS MORE DOCUMENTATION]
 */
class Taskomat
{
public:
    /**
     * Creates a new instance to manage and control of sequences that are serialized to
     * the underlying file system.
     *
     * @param path root path for the serialized sequences.
     */
    explicit Taskomat(gul14::string_view path = "."): path_(path)
    {
    }

    /**
     * Returns the root path of the serialized sequences.
     *
     * @return gul14::string_view root path of the serialized sequences.
     */
    gul14::string_view get_path() const { return path_; }

private:
    /// Root path to the sequences
    gul14::string_view path_;
};

} // namespace task

#endif
