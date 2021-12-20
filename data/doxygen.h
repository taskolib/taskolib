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
 * The evolving design goals of the library are documented in a series of
 * \ref design_documents.
 *
 * \note
 * The Avtomat library requires at least C++17.
 *
 * \authors Lars Froehlich, Olaf Hensler, Marcus Walla (for third-party code see
 *          \ref copyright)
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
 * <dd>Copyright 2021 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
 * This library contains the Sol2 library in the directory src/sol (see
 * https://github.com/ThePhD/sol2). Sol2 is distributed under the MIT license:
 *
 * <dl>
 * <dt>Sol2 library</dt>
 * <dd>Copyright (c) 2013-2021 Rapptz, ThePhD, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</dd>
 * </dl>
 */

/**
 * \page design_documents Design Documents
 *
 * \section evolving_design_documents Evolving Design Documents
 *
 * We are collecting design goals for the library in a series of "evolving design
 * documents". These PDFs also document the evolution of the project over time.
 *
 * <ul>
 * <li><a href="2021-12-17 Evolving Design.pdf">2021-12-17 Evolving Design</a></li>
 * <li><a href="2021-12-10 Evolving Design.pdf">2021-12-10 Evolving Design</a></li>
 * </ul>
 */
