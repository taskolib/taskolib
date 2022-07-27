/**
 * \file   SequenceManager.h
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

#ifndef TASKOMAT_SEQUENCEMANAGER_H_
#define TASKOMAT_SEQUENCEMANAGER_H_

#include <vector>
#include <gul14/string_view.h>
#include "taskomat/Sequence.h"

namespace task {

/**
 * A class to have a birds eye view on the underlying serialized sequences in the file
 * system. It allows to manage and control sequences.
 *
 * Per default the root sequence path is set to '.'.
 *
 * Since we have a predefined flat struture for serialize sequences we come to the
 * assumption for the following specification:
 *
 * \code
 * ./sequence_1 <- folder name that represents the first sequence
 *    step_action_01.lua <- first step of sequence 1
 *    ...
 *    step_action_<n>.lua <- n-th step of sequence 1
 * ...
 * ./sequence_<m> <- folder name that represents the m-th sequence
 *    step_action_01.lua <- first step of sequence 1
 *    ...
 *    step_action_<n>.lua <- n-th step of sequence 1
 * \endcode
 *
 * As a root path we have the current relative folder '.'.
 *
 * [NOTE: NEEDS MORE DOCUMENTATION]
 */
class SequenceManager
{
public:
    using SequenceList = std::unordered_map<std::string, Sequence>;
    using SequenceManagerRef = SequenceManager&;

    /**
     * Get a singleton Taskomat instance.
     *
     * @return SequenceManagerRef as one instance.
     */
    static SequenceManagerRef get()
    {
        // This is:
        // - guaranteed to be destroyed
        // - instantiated on first use
        // - thread safe since C+11
        static SequenceManager instance{}; // ...{}: initialize member variables
        return instance;
    }

    /**
     * Returns the root path of the serialized sequences.
     *
     * @return gul14::string_view root path of the serialized sequences.
     */
    gul14::string_view get_path() const { return path_; }

    /**
     * Set root path for the serialize sequences.
     *
     * @param path new root path.
     */
    void set_path(gul14::string_view path) { path_ = path; }

private:
    /// Root path to the sequences
    gul14::string_view path_{"."};

    /**
     * Creates a new instance to manage and control of sequences that are serialized in
     * the underlying file system.
     */
    explicit SequenceManager() = default;

    /// Destructor.
    ~SequenceManager() = default;

    /// Copy constructor is disabled.
    SequenceManager(const SequenceManager&) = delete;

    /// Copy assignment operator is disabled.
    SequenceManager& operator=(const SequenceManager&) = delete;

    // Note: since we cannot use move semantics with the move constructor or move
    // assignment operator there is no need to declare it here, at least it will throw a
    // compile error (https://en.cppreference.com/w/cpp/language/move_constructor):
    //SequenceManager(const SequenceManager&&) = delete;
    //SequenceManager& operator=(const SequenceManager&&) = delete;
};

} // namespace task

#endif
