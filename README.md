# The Goo [[#](#the-goo)]

This library is designed as a toolkit for scientific application development
needs. This library has a bunch of loosely-bound tools like application class
or various configuration templates for fast assembly of short and versatile
embeddable apps. One may think about `Goo` as of software glue laying between
fast C/C++ routines and higher level steering applications.

The main purpose of this library is to provide some common but useful
versatile infrastructural classes, so one may think as it is a micro-framework
for organizing scientific applications. It does provide a common exception
classes suitable for a number of generic cases with handy stack-trace mechanism
as well as a few advanced error-reporting features: object-oriented signal
handling, run-time `gdb` accompaniment and so on.
# Brief installation notes [[#](#brief-install)]

If you have found this library as a dependency, you will probably wish to build
and install it in user-space (meaning NOT system-wide installation).

## User-space installation

At this case you have to choose a proper location for library to be installed
(let's say at the /opt/goo) and perform an ordinary installation procedure for
CMake project:

* First, clone sources into temporary directory:
    $ cd /tmp
    $ git clone https://github.com/CrankOne/Goo.git
* Then create a separate temporary build directory where intermediate build
files have to be placed:
    $ mkdir -p goo.build/debug
    $ cd goo.build/debug
* Configure and build the library with:
    $ cmake ../../goo -DCMAKE_INSTALL_PREFIX=/opt/goo
    $ make
* ...and install it with:
    $ make install

The `/tmp/goo` sources directory and `/tmp/goo.build` temporary directory can
be then safely removed.

## System-wide installation

System-wide installation is quite similar to installation in userspace except
for `-DCMAKE_INSTALL_PREFIX=...` command-line argument that has to be omitted.
In that case installation prefix will be set to `/usr/local` which is standard
for Linux systems.

# Features branches [[#](#other-branches)]

Some major features are planned to be added into library in the future. This
features usually have a dedicated branches where their development takes place
aside from `master` and `development` branches:

   1. `grammar` branch contains the GDS (Goo Declarative Semantics)
      language. Kind of laconic declarative language of common purposes.
   2. `dataStreams` is a template module that utilizes some brand new
      C++11 features making array marschalling and serialization routines
      pretty much easier and well-readable.
   3. `tensors` is a template module that represents auxilliary stuff
      providing tensorial computations: multivariate arrays, indexing,
      co-/contravariant tensors and so on.
   4. `appParameters` is a parameters tree library implementing a yet another
      argc/argv-parsing facility with ability to merge parameters with config
      files, lexical casts (in relation with GDS). Note: this branch is
      currently merged into `development` and passes pre-release tests.
   5. `allocators` is a memory management subsystem, supplementing standard
      library allocators.

We're working on those features mostly at spare time on a whim.

# License [[#](#license-info)]

> Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>,
>                    Bogdan Vasilishin <togetherwithra@gmail.com>
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy of
> this software and associated documentation files (the "Software"), to deal in
> the Software without restriction, including without limitation the rights to
> use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
> the Software, and to permit persons to whom the Software is furnished to do so,
> subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
> FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
> COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
> IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
> CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

All non-contributed code here can be distributed by the terms of MIT
license and thereby are free for commercial use.
