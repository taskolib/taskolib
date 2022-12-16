/**
 * \mainpage
 *
 * \image html sequence_and_steps.png
 *
 * Taskolib is a library for automating processes. Its main automatization unit is a
 * sequence of steps which are executed in order or through control flow statements. The
 * behavior of each step is defined in the Lua scripting language.
 *
 * The library provides the main modeling classes for sequences and steps, functionality
 * for executing them in the current thread or in a parallel one, as well as serialization
 * support for saving and loading them.
 *
 * \section run_sequence_in_current_thread Create a sequence and run it in the current thread
 *
 * The C++ classes for modeling sequences and steps are \ref task::Sequence "Sequence" and
 * \ref task::Step "Step". A sequence acts like a container for steps and can be executed
 * in the current thread with the member function
 * \ref task::Sequence::execute "Sequence::execute()":
 *
 * \code {.cpp}
 * #include <taskolib/taskolib.h>
 *
 * using namespace task;
 *
 * Sequence create_sequence()
 * {
 *     // Create some steps
 *     Step step1{ Step::type_action };
 *     step1.set_script("a = 42");
 *
 *     Step step2{ Step::type_action };
 *     step2.set_script("print('Mary had a little lamb.')");
 *
 *     // Assemble the steps into a sequence
 *     Sequence sequence{ "An example sequence" };
 *     sequence.push_back(step1);
 *     sequence.push_back(step2);
 *
 *     return sequence;
 * }
 *
 * Sequence sequence = create_sequence();
 * Context context; // A context may contain additional information for the sequence
 *
 * // Run the sequence
 * sequence.execute(context);
 * \endcode
 *
 * \section run_sequence_parallel Run a sequence in a parallel thread
 *
 * Taskolib can start a sequence in a parallel thread via an \ref task::Executor "Executor"
 * object. The main thread should frequently call \ref task::Executor::update "update()"
 * on this object to get information about the status of the execution. No other
 * synchronization is necessary.
 *
 * \code {.cpp}
 * #include <taskolib/taskolib.h>
 *
 * using namespace task;
 *
 * Executor ex;
 * Sequence sequence = create_sequence(); // get a Sequence from somewhere
 * Context context;
 *
 * // Start executing a copy of the sequence in a separate thread
 * ex.run_asynchronously(sequence, context);
 *
 * // Periodically call update() to bring our local copy of the sequence in sync with the
 * // other thread. Once the sequence has finished, update() returns false and the thread
 * // is joined automatically.
 * while (ex.update(sequence))
 *     sleep(0.1s);
 * \endcode
 *
 * \section usage Usage Notes
 *
 * Taskolib requires at least C++17. All functions and classes are declared in the
 * namespace \ref task. To use the library, include the single header file
 * \link taskolib/taskolib.h \endlink and link your code against both this library and
 * the General Utility Library (-ltaskolib -lgul14).
 *
 * The evolving design goals of the library are documented in a series of
 * \ref design_documents.
 *
 * \authors Lars Fröhlich, Olaf Hensler, Ulf Fini Jastrow, Marcus Walla (for third-party
 *          code see \ref copyright)
 *
 * \copyright
 * See \ref copyright
 */

/**
 * \page copyright Copyright Notices
 *
 * \image html desy.png
 *
 * \section copyright_notice Copyright Notice (LGPLv2.1)
 *
 * Taskolib is free software distributed under the terms of the
 * <a href="https://opensource.org/licenses/LGPL-2.1">GNU Lesser General Public License
 * version 2.1</a>. All files except those listed under \ref additional_copyright_notices
 * below are copyrighted by DESY:
 *
 * <dl>
 * <dt>Taskolib</dt>
 * <dd>Copyright 2021-2022 Deutsches Elektronen-Synchrotron (DESY), Hamburg
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
 * This library contains a version of <a href="https://www.lua.org/">Lua</a> in the
 * directory src/lua and a version of the <a href="https://github.com/ThePhD/sol2">Sol2</a>
 * library in the directory src/sol. Sol2 incorporates other open source software such as
 * the Ogonek library. Here are all of the copyright statements and licenses of the
 * individual components:
 *
 * <dl>
 * <dt>Lua</dt>
 * <dd>Copyright (c) 1994–2021 Lua.org, PUC-Rio.
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
 * </dd>
 *
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
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * </dd>
 *
 * <dt>Lua-compat-5.3</dt>
 * <dd>Copyright (c) 2018 Kepler Project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * </dd>
 * </dt>
 *
 * <dt>Ogonek</dt>
 * <dd>Creative Commons Legal Code
 *
 * CC0 1.0 Universal
 *
 *     CREATIVE COMMONS CORPORATION IS NOT A LAW FIRM AND DOES NOT PROVIDE
 *     LEGAL SERVICES. DISTRIBUTION OF THIS DOCUMENT DOES NOT CREATE AN
 *     ATTORNEY-CLIENT RELATIONSHIP. CREATIVE COMMONS PROVIDES THIS
 *     INFORMATION ON AN "AS-IS" BASIS. CREATIVE COMMONS MAKES NO WARRANTIES
 *     REGARDING THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS
 *     PROVIDED HEREUNDER, AND DISCLAIMS LIABILITY FOR DAMAGES RESULTING FROM
 *     THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS PROVIDED
 *     HEREUNDER.
 *
 * Statement of Purpose
 *
 * The laws of most jurisdictions throughout the world automatically confer
 * exclusive Copyright and Related Rights (defined below) upon the creator
 * and subsequent owner(s) (each and all, an "owner") of an original work of
 * authorship and/or a database (each, a "Work").
 *
 * Certain owners wish to permanently relinquish those rights to a Work for
 * the purpose of contributing to a commons of creative, cultural and
 * scientific works ("Commons") that the public can reliably and without fear
 * of later claims of infringement build upon, modify, incorporate in other
 * works, reuse and redistribute as freely as possible in any form whatsoever
 * and for any purposes, including without limitation commercial purposes.
 * These owners may contribute to the Commons to promote the ideal of a free
 * culture and the further production of creative, cultural and scientific
 * works, or to gain reputation or greater distribution for their Work in
 * part through the use and efforts of others.
 *
 * For these and/or other purposes and motivations, and without any
 * expectation of additional consideration or compensation, the person
 * associating CC0 with a Work (the "Affirmer"), to the extent that he or she
 * is an owner of Copyright and Related Rights in the Work, voluntarily
 * elects to apply CC0 to the Work and publicly distribute the Work under its
 * terms, with knowledge of his or her Copyright and Related Rights in the
 * Work and the meaning and intended legal effect of CC0 on those rights.
 *
 * 1. Copyright and Related Rights. A Work made available under CC0 may be
 * protected by copyright and related or neighboring rights ("Copyright and
 * Related Rights"). Copyright and Related Rights include, but are not
 * limited to, the following:
 *
 *   i. the right to reproduce, adapt, distribute, perform, display,
 *         communicate, and translate a Work;
 *  ii. moral rights retained by the original author(s) and/or performer(s);
 * iii. publicity and privacy rights pertaining to a person's image or
 *         likeness depicted in a Work;
 *  iv. rights protecting against unfair competition in regards to a Work,
 *         subject to the limitations in paragraph 4(a), below;
 *   v. rights protecting the extraction, dissemination, use and reuse of data
 *         in a Work;
 *  vi. database rights (such as those arising under Directive 96/9/EC of the
 *         European Parliament and of the Council of 11 March 1996 on the legal
 *         protection of databases, and under any national implementation
 *         thereof, including any amended or successor version of such
 *         directive); and
 * vii. other similar, equivalent or corresponding rights throughout the
 *         world based on applicable law or treaty, and any national
 *         implementations thereof.
 *
 * 2. Waiver. To the greatest extent permitted by, but not in contravention
 * of, applicable law, Affirmer hereby overtly, fully, permanently,
 * irrevocably and unconditionally waives, abandons, and surrenders all of
 * Affirmer's Copyright and Related Rights and associated claims and causes
 * of action, whether now known or unknown (including existing as well as
 * future claims and causes of action), in the Work (i) in all territories
 * worldwide, (ii) for the maximum duration provided by applicable law or
 * treaty (including future time extensions), (iii) in any current or future
 * medium and for any number of copies, and (iv) for any purpose whatsoever,
 * including without limitation commercial, advertising or promotional
 * purposes (the "Waiver"). Affirmer makes the Waiver for the benefit of each
 * member of the public at large and to the detriment of Affirmer's heirs and
 * successors, fully intending that such Waiver shall not be subject to
 * revocation, rescission, cancellation, termination, or any other legal or
 * equitable action to disrupt the quiet enjoyment of the Work by the public
 * as contemplated by Affirmer's express Statement of Purpose.
 *
 * 3. Public License Fallback. Should any part of the Waiver for any reason
 * be judged legally invalid or ineffective under applicable law, then the
 * Waiver shall be preserved to the maximum extent permitted taking into
 * account Affirmer's express Statement of Purpose. In addition, to the
 * extent the Waiver is so judged Affirmer hereby grants to each affected
 * person a royalty-free, non transferable, non sublicensable, non exclusive,
 * irrevocable and unconditional license to exercise Affirmer's Copyright and
 * Related Rights in the Work (i) in all territories worldwide, (ii) for the
 * maximum duration provided by applicable law or treaty (including future
 * time extensions), (iii) in any current or future medium and for any number
 * of copies, and (iv) for any purpose whatsoever, including without
 * limitation commercial, advertising or promotional purposes (the
 * "License"). The License shall be deemed effective as of the date CC0 was
 * applied by Affirmer to the Work. Should any part of the License for any
 * reason be judged legally invalid or ineffective under applicable law, such
 * partial invalidity or ineffectiveness shall not invalidate the remainder
 * of the License, and in such case Affirmer hereby affirms that he or she
 * will not (i) exercise any of his or her remaining Copyright and Related
 * Rights in the Work or (ii) assert any associated claims and causes of
 * action with respect to the Work, in either case contrary to Affirmer's
 * express Statement of Purpose.
 *
 * 4. Limitations and Disclaimers.
 *
 *  a. No trademark or patent rights held by Affirmer are waived, abandoned,
 *     surrendered, licensed or otherwise affected by this document.
 *  b. Affirmer offers the Work as-is and makes no representations or
 *     warranties of any kind concerning the Work, express, implied,
 *     statutory or otherwise, including without limitation warranties of
 *     title, merchantability, fitness for a particular purpose, non
 *     infringement, or the absence of latent or other defects, accuracy, or
 *     the present or absence of errors, whether or not discoverable, all to
 *     the greatest extent permissible under applicable law.
 *  c. Affirmer disclaims responsibility for clearing rights of other persons
 *     that may apply to the Work or any use thereof, including without
 *     limitation any person's Copyright and Related Rights in the Work.
 *     Further, Affirmer disclaims responsibility for obtaining any necessary
 *     consents, permissions or other rights required for any use of the
 *     Work.
 *  d. Affirmer understands and acknowledges that Creative Commons is not a
 *     party to this document and has no duty or obligation with respect to
 *     this CC0 or use of the Work.
 * </dd>
 *
 * <dt>Allow string literals as case labels</dt>
 * <dd>Copyright 2020-2022 2020-22 Richard Spencer
 *
 * MIT License
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
 * </dd>
 *
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
 * <li><a href="2022-01-07 Evolving Design.pdf">2022-01-07 Evolving Design</a></li>
 * <li><a href="2022-04-04 Evolving Design.pdf">2022-04-04 Evolving Design</a></li>
 * </ul>
 */
