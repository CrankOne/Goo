*This readme is realated only for generic (master) branch of Goo library.*

# The Goo [[#](#the-goo)]

This library composes together some stuff that was done during various
scientific application development. The need in such an infrastructural library
comes from modern C/C++ applications requirements.

We have arranged various loosely-bound elements in modular manner instead of
making something with centralized framwork architecture and trying to handle
here only uncommon or poor-implemented things. Actually, many of them (e.g.
`boost::variables_map`) were just found inconvinient in various practical cases.
The Goo offers a replacement for them.

# Brief installation notes [[#](#brief-install)]

If one had come to this page, this probably means that this library is need
as a dependency since we often include Goo into other projects. If so, we would
recommend to use this snippet for quick build and non-obtrusive installation:

    $ cd /tmp
    $ git clone https://github.com/CrankOne/Goo.git
    $ mkdir -p goo.build/debug
    $ cd goo.build/debug
    $ cmake ../../goo
    $ make
    $ make DESTDIR=/opt/goo install

One can choose any path for DESTDIR where goo library would be installed. Just
be sure you can point out this directory for subsequent build operations.

## Master branch description [[#](#branch-description)]

Each module here usually has its own branch. The `master`
branch is intended to be only compilative all-in-one branch for putting
the modules altogether.

The `master` branch contains only generic application framework library for
all descendant branches: application class, exceptions and modest unit-testing
facility.

## Other branches [[#](#other-branches)]

Excluding several initial commits, we're planning to develop the following
things in parallel before thhe first release at `master branch`:

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

One can not probably find all of them at my [github repo](https://github.com/CrankOne/Goo)
whilst we working on those features mostly at the spare time on a whim.

# License [[#](#license-info)]

All non-contributed code here can be distributed by the terms of MIT
license and thereby are free for commercial use.
