# The `goo:dict` template library

Any defined template class `X` (say, `template<typename T> class X`) defines
an unlimited set of possible derived implementations (`X<int>`,
`X<std::string>`, etc.).

The `goo::dict` template helpers tries to define a generalized container
classes allowing to store and enumerate instances of such superset, relying on
modern C++ template specialization mechanics (C++11 and hiegher).

For this become possible we do define the following auxilliary type traits
subsystem.

## `ReferableTraits` specification

Since we're defining the collection of entities that may be addressed
individually within the aggregating container, we will now consider the
"referable" entities meaning the class wrapping target instance of `X<T>`.

A "referable" class objects:
   1. contains the single particular `X<T>` entity
   2. defines data access operations
   3. establishes minimal introspection necessary within `goo::dict`
   4. has common type-agnostic base class unifying operations with set of
     referables
   5. has a specification for the nested (subsequent) collection type
     (referable dictionary)

Thus, the traits defined by `X` class template shall define at least three
classes within:
   * the abstract base referable class (`ReferableBase` on
     diagram)
   * the referable template wrapper class itself (`ReferableMessenger`)
     inherited from abstract base referable class
   * the special case of subsequent indexed collection of referables
     (`DictionaryMessenger`) offering additional querying/type casting
     rotuines specific for the referable collections

`ReferableTraits` struct also defines the allocator appropriate for template
superset `X<T>`. For that there is a technical reason only: despite keeping the
allocator as a template parameter had became a common practice in C++,
introducing it in every template in `goo::dict` drastically increases
complexity of the code with obvious redundancy. At the same time, keeping it
within `ReferableTraits` seems to offer a reasonable flexibility by still
providing a sufficient variety of customization capabilities.

## `ContainerTraits` specification

Typical operations over the enumerated collections (arrays, lists, hash maps,
trees), required for the relevant task are summarized within `ContainerTraits`
specification. As the most versatile specification, the `std::unordered_map`
was taken as the base for the `ContainerTraits` representation. The
`DictionaryMessenger` default template highly relies on these most frequently
used operations (like `find()` or `emplace()`) and types (`iterator`,
`reference`, etc.)

## The `::goo::dict::generic` namespace

As the `using` keyword has became available for aliasing templates (starting
from C++11), combining of the default template implementations with custom
ones in a frame of single `traits` object become an quite expressive solution.
The `goo::dict` offers a series of default templates for the referable classes
and `ContainerTraits` that may for initial basis for user applications.

