# include "gds/arithmetic_eval.h"
# include <unordered_map>

static_assert(sizeof( BinOpKey ) >= 2*sizeof(TypeCode) + sizeof(BinaryArithOpCode),
        "Binary arithmetic indexing type is smaller than need.");

typedef std::unordered_map<BinOpKey, std::pair<TypeCode, BinaryOperatorFunction> > BOTable;

extern "C" GDS_BinOpArithmetic *
gds_alloc_binop_table() {
    GDS_BinOpArithmetic * r = (GDS_BinOpArithmetic *) malloc(sizeof(GDS_BinOpArithmetic));
    r->hashTable = new BOTable();
    return r;
}

# define TABLE(a, b) BOTable * b = reinterpret_cast<BOTable*>(a->hashTable)

extern "C" void
gds_free_binop_table( struct GDS_BinOpArithmetic * tableStruct ) {
    TABLE( tableStruct, table );
    delete table;
    tableStruct->hashTable = NULL;
    free( tableStruct );
}

extern "C" int
gds_add_binary_operator_table_entry(
        struct GDS_BinOpArithmetic * tableStruct,
        BinaryArithOpCode binOpCode,
        TypeCode leftT, TypeCode rightT, TypeCode resultT,
        BinaryOperatorFunction func,
        uint8_t doOverride ) {
    TABLE( tableStruct, table );
    bool doRepeatInsertion = false,
         overriden = false;
    do {
        doRepeatInsertion = false;
        auto insertionResult = table->insert(
                std::pair< BinOpKey, std::pair<TypeCode, BinaryOperatorFunction> >(
                    gds_compose_binop_key( leftT, rightT, binOpCode ),
                    std::pair<TypeCode, BinaryOperatorFunction>( resultT, func )
                )
            );
        if( !insertionResult.second ) {
            if( insertionResult.first->second.second == func ) {
                return 1;
            }
            if( doOverride ) {
                doRepeatInsertion = true;
                overriden = true;
                table->erase( insertionResult.first );
            } else {
                return -1;
            }
        } else {
            return overriden ? 2 : 0;
        }
    } while(doRepeatInsertion);
    return -3;
}

