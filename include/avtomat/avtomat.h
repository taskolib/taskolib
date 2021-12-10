/**
 * \file   avtomat.h
 * \author Lars Froehlich
 * \date   Created on November 26, 2021
 * \brief  Main include file for the Avtomat Library.
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

/**
 * \mainpage
 *
 * \image html desy.png
 *
 * The Avtomat library helps to automate processes. Its main automatization unit is a
 * sequence of steps which are executed in order or through control flow statements. The
 * behavior of each step is defined in the LUA scripting language. The library uses the
 * namespace \ref avto.
 *
 * To use the library, include the single header file \link avtomat/avtomat.h \endlink and
 * link your code against both this library and the General Utility Library (-lavtomat
 * -lgul14).
 *
 * \note
 * The HLC utility library requires at least C++14.
 *
 * \author Lars Froehlich (for third-party code see \ref copyright)
 *
 * \copyright
 * See \ref copyright
 */

/**
 * \page copyright Copyright Notices
 *
 * \section copyright_notice Copyright Notice (LGPLv2.1)
 *
 * The following license applies to all library files with the exception of those listed
 * under \ref additional_copyright_notices below.
 *
 * <dl>
 * <dt>Code contributed by DESY</dt>
 * <dd>Copyright 2015-2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.</dd>
 * </dl>
 *
 * \section additional_copyright_notices Additional Copyright Notices
 *
 * This library currently contains no third-party code.
 */

#ifndef AVTOMAT_AVTOMAT_H_
#define AVTOMAT_AVTOMAT_H_

#include "avtomat/Error.h"
#include "avtomat/LuaState.h"
#include "avtomat/Step.h"

/// Namespace avto contains all functions and classes of the Avtomat libs.
namespace avto { }

#endif
