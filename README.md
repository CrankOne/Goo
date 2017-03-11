*This readme is realated only for generic (master) branch of Goo library.*

# The Goo [[#](#the-goo)]

This library composes together some stuff I've done during my
scientific application development. I'm trying to follow a strict
coding convention when dealing with C/C++ and need a central
framework to embed other stuff in.

I'm arranging this in modular way instead of making something with
single concept and trying to handle only uncommon or
poor-implemented things. Actually, many of them (e.g.
boost::variables\_map) were just found inconvinient to me, so I'd
prefer to develop my own replacement.

# Brief installation notes [[#](#brief-install)]

If one came to this page, it most probably means that this library is need
as a dependency since I often include it into other projects. If so, I would
recomment to use this snippet for quick build and non-obtrusive installation:

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

Each module I develop here usually has its own branch. The `master`
branch is intended to be only compilative all-in-one branch for putting
the modules altogether.

The `master` branch contains only generic application framework library for
all descendant branches: application class, exceptions and modest unit-testing
facility.

## Other branches [[#](#other-branches)]

Excluding several initial commits, I've planning to develop the
following things in parallel before thhe first release at
`master branch`:

   1. `grammar` branch contains the GDS (Goo Declarative Semantics)
      language. Kind of laconic declarative language of common purposes.
   2. `dataStreams` is a template module that utilizes some brand new
      C++11 features making array marschalling and serialization routines
      pretty much easier and well-readable.
   3. `tensors` is a template module that represents auxilliary stuff
      providing tensorial computations: multivariate arrays, indexing,
      co-/contravariant tensors and so on.
   4. `parTree` is a parameters tree library implementing a yet another
      argc/argv-parsing facility with ability to merge parameters with config
      files, lexical casts (in relation with GDS).
   5. `allocators` is a memory management subsystem, supplementing standard
      library allocators.

One can not probably find all of them at my [github repo](https://github.com/CrankOne/Goo)
whilst I'm working on those features mostly at my spare time on a whim.

# License [[#](#license-info)]

All non-contributed code here can be distributed by the terms of MIT
license and thereby are free for commercial use.
