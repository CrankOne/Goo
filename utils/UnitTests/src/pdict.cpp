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

/*
 * Created by crank on 16.01.18.
 */

/**\file pdict.cpp
 *
 * Generic goo::dict framework testing unit.
 * */

# include "utest.hpp"
# include "goo_dict/generic_dict.tcc"
# include "goo_dict/common_aspects.hpp"
# include "goo_dict/util/subsections.tcc"

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

template<typename T> struct SimpleValueKeeper {
    T value;
};

# if 0

namespace goo {
namespace dict {

}  // namespace ::goo::dict
}  // namespace ::goo

# endif

static const char _local_tstText[] = R"Kafka(
coal all spent the bucket empty the shovel useless the stove breathing out
cold the room freezing the trees outside the window rigid covered with
rime the sky a silver shield against anyone who looks for help from it i
must have coal i cannot freeze to death behind me is the pitiless stove
before me the pitiless sky so i must ride out between them and on my journey
seek aid from the coal dealer but he has already grown deaf to ordinary
appeals i must prove irrefutably to him that i have not a single grain of
coal left and that he means to me the very sun in the firmament i must
approach like a beggar who with the death rattle already in his throat
insists on dying on the doorstep and to whom the cook accordingly decides to
give the dregs of the coffeepot just so must the coal dealer filled with
rage but acknowledging the command thou shalt not kill fling a shovelful
of coal into bucket

my mode of arrival must decide the matter so i ride off on the bucket
seated on the bucket my hands on the handle the simplest kind of bridle
i propel myself with difficulty down the stairs but once downstairs my bucket
ascends superbly superbly camels humbly squatting on the ground do not rise
with more dignity shaking themselves under the sticks of their drivers
through the hard‐frozen streets we go at a regular canter often i am upraised
as high as the first story of a house never do i sink as low as the house
doors and at last i float at an extraordinary height above the vaulted cellar
of the dealer whom i see far below crouching over his table where he is
writing he has opened the door to let out the excessive heat
)Kafka";

# if 0
static void
fill_txt( const char * text
        , ::goo::dict::WordInsertionProxy & ip ) {
    const char * wordBegin = nullptr;
    for( const char * c = text; '\0' != *c ; ++c ) {
        if( isalnum(*c) ) {
            if( !wordBegin ) {
                wordBegin = c;
            }
            continue;
        } else if( wordBegin ) {
            ip.consider( wordBegin, c );
        }
    }
}
# endif

GOO_UT_BGN( PDict, "Parameters dictionary routines" ) {
    ::goo::dict::Dictionary<char, SimpleValueKeeper, std::allocator> d;
    //fill_txt( _local_tstText
    //        , d.insertion_proxy() );
} GOO_UT_END( PDict, "VCtr" )

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)
