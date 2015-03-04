# include "utest.hpp"
# include "streams/hph_stream.hpp"
# include "streams/resources/hph_mem.hpp"
//# include "hph_serialization.hpp"

static unsigned short testingArray[2][4][3] = { 
    {{111, 112, 113}, {121, 122, 123}, {131, 132, 133}, {141, 142, 143}},
    {{211, 212, 213}, {221, 222, 223}, {231, 232, 233}, {241, 242, 243}},
}, testingArrayCheck[2][4][3];

static unsigned short testingCheckSequence[] = {
    111, 112, 113, 121, 122, 123, 131, 132, 133, 141, 142, 143,
    211, 212, 213, 221, 222, 223, 231, 232, 233, 241, 242, 243,
};

static void unwinding_tester( unsigned short val ) {
    static unsigned short indexCounter = 0;
    _ASSERT( testingCheckSequence[indexCounter++] == val,
             "Check sequence failed: unwinding order unexpected or malfunctional %d != %d(%d).",
             (int) val,
             (int) testingCheckSequence[indexCounter],
             (int) indexCounter );
}

//
// Unit
//

HPH_UT_BGN( Serialization )

// 1. Test ary unwinding
auto nary = hph::stream::ary_for_array( testingArray );
hph::stream::aux::AryExtentUnwinder<DECLTYPE(testingArray) /*unsigned short[2][4][3]*/,
                                    void (&)(unsigned short), 3, 3>::unwind( testingArray,
                                                                             unwinding_tester,
                                                                             nary );
// 2. Test streams serialization ability.
hph::stream::resources::RawMemoryInputList memR;
hph::stream::Stream<
        hph::stream::Direction::in,
        hph::stream::Access::serial,
        hph::stream::Introspectivity::none
    > bsi(memR);
bsi << 42;
bsi << testingArray;

// 3. Dump resource state.
memR.state_report( out );

// 4. Test steams deserialization.
# if 0
hph::stream::Stream<
        hph::stream::Direction::out,
        hph::stream::Access::serial,
        hph::stream::Introspectivity::none
    > bso(memR);

int fortyTwo;

bso >> fortyTwo;
_ASSERT( 42 == fortyTwo, "Single variable restored incorrectly: %d != %d.", 42, fortyTwo );
bso >> testingArrayCheck;
for( UByte i = 0; i < 2; ++i ) {
    for( UByte j = 0; j < 4; ++j ) {
        for( UByte k = 0; k < 3; ++k ) {
            _ASSERT( testingArray[i][j][k] == testingArrayCheck[i][j][k],
                    "Array restored data is malformed. Mismatch found at [%d,%d,%d]: %d != %d",
                    (int) i, (int) j, (int) k,
                    testingArray[i][j][k], testingArrayCheck[i][j][k] );
        }
    }
}
# endif

HPH_UT_END( Serialization )

# if 0

// Has default ctr.
class SerTest : public ::hph::mixins::iUniformSerializable<2*sizeof(int) + sizeof(float)> {
protected:
    int a, b;
    float c;
    virtual void _V_serialize( ::hph::iByteStreamIn && bsi ) const override {
        bsi << a << b << c;
    }
    virtual void _V_deserialize( ::hph::iByteStreamOut && bso ) override {
        bso >> a >> b >> c;
    }
public:
    SerTest( int a_, int b_, float c_ ) : a(a_), b(b_), c(c_) {}
    SerTest( ) : a(-1), b(-1), c(-1) {}

    void set( int a_, int b_, float c_ ) { a = a_; b = b_; c = c_; }
    // TODO: deserialization ctr
};

// Has no default ctr -- contains a constant and a reference.
class SerTestNontrivial : public ::hph::mixins::iVariadicSerializable {
protected:
    const char _one;  // set by ctr
    int      & _two;  // set by ctr
    float _three;     // reconstructed
    Size    _sz;
    float * _array;

    virtual Size _V_serialized_size( ) const override {
        return sizeof(float)
               + sizeof(Size)
               + _sz*sizeof(float);
    }
    virtual void _V_serialize( ::hph::iByteStreamIn && bsi ) const override {
        bsi << _three << _sz;
        for( Size i = 0; i < _sz; ++i ) {
            bsi << _array[i];
        }
    }
    virtual void _V_deserialize( ::hph::iByteStreamOut && bso ) override {
        bso >> _three >> _sz;
        _array = new float [_sz];
        for( Size i = 0; i < _sz; ++i ) {
            bso >> _array[i];
        }
    }
public:
    SerTestNontrivial( char one, int & two ) : _one(one), _two(two) {
        ++_two;
        _sz = 5 + 10*double(rand())/RAND_MAX;
        _array = new float [_sz];
        for( Size i = 0; i < _sz; ++i ) {
            _array[i] = 10*double(rand())/RAND_MAX;
        }
    }
    ~SerTestNontrivial() { delete [] _array; --_two; }
    // TODO: deserialization ctr
};

static short integers[] = { 0xDE, 0xAD, 0xFA, 0xCE };
static const uint8_t nInts = sizeof(integers)/sizeof(short);
static float floats[]   = { 1.23, 15.67, 19.84 };
static const uint8_t nFloats = sizeof(floats)/sizeof(float);
static long multiVarArray[5][3][4][7], multiVarArrayCheck[5][3][4][7];
static SerTest tstPODLike[3][2][4];

void fill_mvar_array() {
    for( uint8_t i = 0; i < 5; ++i ) {
        for( uint8_t j = 0; j < 3; ++j ) {
            for( uint8_t k = 0; k < 4; ++k ) {
                for( uint8_t l = 0; l < 7; ++l ) {
                    multiVarArray[i][j][k][l] =
                        l + k*10 + j*100 + i*1000;
                }
            }
        }
    }
    for( uint8_t i = 0; i < 3; ++i ) {
        for( uint8_t j = 0; j < 2; ++j ) {
            for( uint8_t k = 0; k < 4; ++k ) {
                tstPODLike[i][j][k].set( i, j, k );
            }
        }
    }
}

HPH_UT_BGN( Serialization )

srand( time(NULL) );
fill_mvar_array();

out << hph::mem::ary(5,3,4,7) << std::endl;

// Declare non-trivial structs:
int ntObjs = 0;
SerTestNontrivial a( 'a', ntObjs ),
                  varLArray[4][2][3] = {
        { {{'b', ntObjs}, {'c', ntObjs}, {'d', ntObjs}}, {{'e', ntObjs}, {'f', ntObjs}, {'g', ntObjs}} },
        { {{'h', ntObjs}, {'i', ntObjs}, {'j', ntObjs}}, {{'k', ntObjs}, {'l', ntObjs}, {'m', ntObjs}} },
        { {{'n', ntObjs}, {'o', ntObjs}, {'p', ntObjs}}, {{'q', ntObjs}, {'r', ntObjs}, {'s', ntObjs}} },
        { {{'t', ntObjs}, {'u', ntObjs}, {'v', ntObjs}}, {{'w', ntObjs}, {'x', ntObjs}, {'y', ntObjs}} },
    };

//const char outFName[] = "/tmp/SerializationTest.hsd";

//
// SERIALIZATION
//

// Uncomment below to disable particular serialization techniques:
//# define DISABLE_SINGL_ATOMIC
//# define DISABLE_ARRAY_ATOMIC
//# define DISABLE_MULTV_ATOMIC
//# define DISABLE_SINGL_UNIFOR
//# define DISABLE_MULTV_UNIFOR
//# define DISABLE_SINGL_COMPND
//# define DISABLE_MULTV_COMPND

auto A = hph::mem::get_default_allocator();

{   
    // Declare binary stream to input into as a test:
    hph::mem::ObjStreamIn<> bsIn(A);

    // Serialize some C-arithmetic types and its arrays:
    #if not (defined DISABLE_SINGL_ATOMIC && defined DISABLE_ARRAY_ATOMIC)
    bsIn 
        # ifndef DISABLE_SINGL_ATOMIC
         << 42
         << (float) M_PI
        # endif
        # ifndef DISABLE_ARRAY_ATOMIC
         << hph::mem::ary(nInts)   << integers
         << hph::mem::ary(nFloats) << floats
        # endif
         ;
    # endif
    # ifndef DISABLE_MULTV_ATOMIC
    bsIn << hph::mem::ary(5,3,4,7) << multiVarArray;
    # endif
    # ifndef DISABLE_SINGL_UNIFOR
    // Serialize POD-like (uniform) type and array:
    bsIn << SerTest{ 1, 2, 3.14 };
    # endif
    bsIn.enable_type_hints();
    # ifndef DISABLE_SINGL_ATOMIC
    bsIn << (unsigned long) 0xDEADBEEF;
    # endif
    # ifndef DISABLE_MULTV_UNIFOR
    bsIn << hph::mem::ary(3, 2, 4) << tstPODLike;
    # endif

    // Serialize variadic length data struct:
    #if not (defined DISABLE_SINGL_COMPND && defined DISABLE_MULTV_COMPND)
    bsIn 
        # ifndef DISABLE_SINGL_COMPND
         << a
        # endif
        # ifndef DISABLE_MULTV_COMPND
         << hph::mem::ary(3, 2, 4) << varLArray
        # endif
         ;
    # endif

    bsIn.disable_type_hints();

    // Dump description of serialized sequence:
    out << bsIn;

    // Write serialized data to buffers.
    hph::mem::DynamicBuffer descriptorsBuffer( bsIn.stat().descriptorsLength, A ),
                              typeHintsBuffer( bsIn.stat().typeHintsLength, A  ),
                                   dataBuffer( bsIn.stat().dataBlockLength, A  );

    // In order to make sure that all buffers are filled without extra bytes
    // remained at the end, we will declare here a pointer variables, while
    // for production one can do just:
    //      bsIn.store_descriptors( ByteStream<...>(descriptorsBuffer) );
    //      bsIn.store_hints( ByteStream<...>(typeHintsBuffer) );
    //      ...
    // If buffers lengths aren't sufficient, it will lead to SEGFAULT most
    // probably... Anyway, these assertions will be failed if something will go
    // wrong.
    UByte * dsPtr = descriptorsBuffer.data(),
          * hdPtr = typeHintsBuffer.data(),
          * dbPtr = dataBuffer.data();

    typedef hph::ByteStream<bstream::Direction::in,
                            bstream::Resource::memory,
                            bstream::Ownership::uses> ibsomem;
    typedef hph::ByteStream<bstream::Direction::out,
                            bstream::Resource::memory,
                            bstream::Ownership::uses> obsomem;

    bsIn.store_descriptors( ibsomem(dsPtr) );
    _ASSERT( dsPtr == descriptorsBuffer.end(),
             "Descriptors buffer has extra %zu bytes at the end.",
             descriptorsBuffer.end() - dsPtr);
    bsIn.store_hints(       ibsomem(hdPtr) );
    _ASSERT( hdPtr == typeHintsBuffer.end(),
             "Type hints buffer has extra %zu bytes at the end.",
             typeHintsBuffer.end() - hdPtr);
    bsIn.store_data(        ibsomem(dbPtr) );
    _ASSERT( dbPtr == dataBuffer.end(),
             "Data buffer has extra %zu bytes at the end.",
             dataBuffer.end() - dbPtr);

    //
    // Deserialization
    //

    // Descriptors checks

    obsomem dataWrapper( dataBuffer );
    hph::mem::ObjStreamOut<bstream::Resource::memory> bsOut(
                obsomem(descriptorsBuffer),
                obsomem(typeHintsBuffer),
                dataBuffer,
                ::hph::mem::get_default_allocator()
            );
    out << bsOut;

    _ASSERT(  bsIn.stat().descriptorsLength == bsOut.stat().descriptorsLength,
              "Descriptors length (stored/restored): " HPH_SIZE_FMT " != " HPH_SIZE_FMT,
              bsIn.stat().descriptorsLength, bsOut.stat().descriptorsLength
            );
    _ASSERT(  bsIn.stat().typeHintsLength   == bsOut.stat().typeHintsLength,
              "Type hints length (stored/restored): " HPH_SIZE_FMT " != " HPH_SIZE_FMT,
              bsIn.stat().typeHintsLength, bsOut.stat().typeHintsLength
            );
    _ASSERT(  bsIn.stat().dataBlockLength   == bsOut.stat().dataBlockLength,
              "Data block length (stored/restored): " HPH_SIZE_FMT " != " HPH_SIZE_FMT,
              bsIn.stat().dataBlockLength, bsOut.stat().dataBlockLength
            );


}

//
// DESERIALIZATION
//

{
    # if 0
    hph::mem::ObjStreamOut<> bsOut(hph::mem::get_default_allocator(),
                                   outFName);
    int intVar;
    short intArrCheck[sizeof(integers)/sizeof(short)];
    float fltVar, fltVarCheck[sizeof(floars)/sizeof(float)];
    bsOut >> intVar;    _ASSERT(           42 == intVar, "Deserialization error #1" );
    bsOut >> fltVar;    _ASSERT( (float) M_PI == fltVar, "Deserialization error #2" );
    bsOut >> intArrCheck;
    for( uint8_t i = 0 i < nInts; ++i ) {
        _ASSERT( intArrCheck[i] == integers[i],
                 "Deserialization error #3");
    }
    bsOut >> fltVarCheck;
    for( uint8_t i = 0 i < nFloats; ++i ) {
        _ASSERT( fltArrCheck[i] == floats[i],
                 "Deserialization error #4");
    }
    bsOut >> multiVarArrayCheck;
    # endif
}

HPH_UT_END( Serialization )

# endif

