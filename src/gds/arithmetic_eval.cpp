# include "gds/arithmetic_eval.h"
# include <unordered_map>

static_assert(sizeof( BinOpKey ) >= 2*sizeof(TypeCode) + sizeof(BinaryArithOpCode),
        "Binary arithmetic indexing type is smaller than need.");

typedef std::unordered_map<BinOpKey, std::pair<TypeCode, BinaryOperatorFunction> > BOTable;

GDS_BinOpArithmetic *
gds_alloc_binop_table() {
    GDS_BinOpArithmetic * r = (GDS_BinOpArithmetic *) malloc(sizeof(GDS_BinOpArithmetic));
    r->hashTable = new BOTable();
    return r;
}

# define TABLE(a, b) BOTable * b = reinterpret_cast<BOTable*>(a->hashTable)

void
gds_free_alloc_binop_table( struct GDS_BinOpArithmetic * tableStruct ) {
    TABLE( tableStruct, table );
    delete table;
    tableStruct->hashTable = NULL;
    free( tableStruct );
}

int
gds_add_binary_operator_table_entry(
        struct GDS_BinOpArithmetic * tableStruct,
        BinaryArithOpCode binOpCode,
        TypeCode leftT, TypeCode rightT, TypeCode resultT,
        BinaryOperatorFunction func,
        uint8_t doOverride ) {
    TABLE( tableStruct, table );
    int r = -3;
    for( r = -3; r < -1; r++ ) {
        auto insertionResult = table->insert(
                std::pair< BinOpKey, std::pair<TypeCode, BinaryOperatorFunction> >(
                    gds_compose_binop_key( leftT, rightT, binOpCode ),
                    std::pair<TypeCode, BinaryOperatorFunction>( resultT, func )
                )
            );
        if( !insertionResult.second ) {
            if( insertionResult.first->second.second == func ) {
                r = 1;
            }
            if( !doOverride ) {
                r = -1;
            }
            table->erase( insertionResult.first );
        } else {
            r = 0;
        }
    }
    return r;
}

