Taskolib Library
================

The Taskolib library helps to automate processes. Its main automatization unit is a
sequence of steps which are executed in order or through control flow statements. The
behavior of each step is defined in the Lua scripting language.

The library API is documented with Doxygen on the web page
https://taskolib.github.io/taskolib/ .

License
-------
The Taskolib library is free software distributed under the terms of the GNU Lesser
General Public License version 2.1 (LGPLv2.1). Included third-party software carries
additional copyright statements and licenses (which are, however, compatible with the
LGPLv2.1). Please refer to the web page or to the source file data/doxygen.h for details.

Building
--------
The library is built with Meson/Ninja. Building is done in two stages: You start by
setting up the build system with a call to Meson. All configurable options are set in this
step. Then, you start the actual build with the ninja command.

To set up a build directory `builddir` with the default configuration and build the
library in it:

      meson builddir
      ninja -C builddir

By convention, directory names starting with `build` or subdirectories of a `build*/` folder
are used.

Debug and Release Builds
------------------------
Meson differentiates between debug and release builds at configuration time (when setting
up the build directory). These are the most often used variants:

      meson --buildtype=debug builddir
      meson --buildtype=debugoptimized builddir
      meson --buildtype=release builddir

Building the Documentation
--------------------------
This library carries documentation embedded in the source code. Run the following tool
to generate HTML documentation:

      ./tools/make-doc.py

This runs the open-source tool Doxygen and generates a web page in the directory
docs/html.

Inspect Parameters
------------------
To see all the possible and the active settings, change into the build directory and call
`meson configure`. You can see some of the built-in possibilities there, like unity builds
and sanitizer to use.

Examples
--------

#### like: make

      meson --buildtype debug builddir

Not optimized build in `builddir/` (optimized for debugging in `gdb` for ex.)

#### like: make install

      meson builddir

Fully optimized build in `builddir/`, installs in `/usr/lib` and
`/usr/lib/include`.

This can be used together with `debuild` to create packages, after
adapting the debian/rules (not recommended, just use the existing rules).
The staging directory will be transfered via `DESTDIR`.

#### like: make localinstall
_(install in `$HOME/???/lib`)_

      meson --prefix ???/somewhere builddir

Installs into the user's home directory.

To use it one has to set `LD_LIBRARY_PATH`, `PKG_CONFIG_PATH`, and `CPATH` (probably).

#### After the setup phase

... one can call any of these:

      ninja -C builddir
      ninja -C builddir test
      ninja -C builddir install
      ninja -C builddir clean
      rm -Rf builddir

