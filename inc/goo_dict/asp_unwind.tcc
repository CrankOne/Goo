/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
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
 */

// Created by crank on 25.02.18.

# ifndef H_GOO_DICT_ASPECTS_UNWINDING_H
# define H_GOO_DICT_ASPECTS_UNWINDING_H

namespace goo {
namespace dict {

class iAbstractValue;
template<typename ... AspectTs> class iBaseValue;

namespace aspects {
namespace aux {

// Recursive template aspects unwinding template.
template< template <typename> class MetaFunctionT
        , int N
        , typename HeadT
        , typename ... TailTs>
struct UnwindingApplier {
    template<typename ... AspectTs>
    static void apply(iAbstractValue * trg, std::tuple<AspectTs * ...> t) {
        MetaFunctionT<HeadT>::Doer::do_( std::get<N>(t), trg );
        UnwindingApplier<MetaFunctionT, N+1, TailTs...>:: \
                template apply<AspectTs...>( trg, t );
        //auto rs[] = {  };
    }
};

// Last aspect unwinding (template arguments expansion terminator).
template< template <typename> class MetaFunctionT
        , int N
        , typename HeadT>
struct UnwindingApplier<MetaFunctionT, N, HeadT> {
    template<typename ... AspectTs>
    static void apply(iAbstractValue * trg, std::tuple<AspectTs * ...> t) {
        MetaFunctionT<HeadT>::Doer::do_( std::get<N>(t), trg );
    }
};

template< template <typename> class MetaFunctionT
        , typename ... AspectTs>
struct ConditionalInTuple {
    static void apply( iBaseValue<AspectTs...> * t ) {
        UnwindingApplier< MetaFunctionT
                        , 0
                        , AspectTs ... >::apply( t, t->aspects() );
    }
};

template< template<typename> class MetaFunctionT>
struct ConditionalInTuple<MetaFunctionT> {
    static void apply( iBaseValue<> * ) {/* do nothing when no aspects are given */}
};

}  // namespace aux
}  // namespace aspects
}  // namespace dict
}  // namespace goo

# endif

