# include "goo_config.h"

# ifdef ENABLE_GDS

# include <unordered_map>
# include <cassert>

# include "gds/goo_interpreter.h"
# include "goo_exception.hpp"

typedef std::unordered_map<std::string, void*> HT;

gds_Hashtable gds_hashtable_new() {
    return new HT();
}

void gds_hashtable_free(        gds_Hashtable hs_ ) {
    assert( hs_ );
    auto hs = reinterpret_cast<HT *>(hs_);
    delete hs;
}

void gds_hashtable_insert(      gds_Hashtable hs_,
                                const char * key,
                                void * data ) {
    assert( data );
    assert( hs_ );
    auto hs = reinterpret_cast<HT *>(hs_);
    if( hs->end() != hs->find(key) ) {
        emraise( nonUniq,
                 "Identifier already known: \"%s\".",
                 key );
    }
    (*hs)[key] = data;
}

void
gds_hashtable_replace(      gds_Hashtable hs_,
                            const char * key,
                            void * data  ) {
    assert( data );
    assert( hs_ );
    auto hs = reinterpret_cast<HT *>(hs_);
    if( hs->end() == hs->find(key) ) {
        emraise( nonUniq,
                 "Identifier is not known and can't be replaced: \"%s\".",
                 key );
    }
    (*hs)[key] = data;
}

void * gds_hashtable_search(    gds_Hashtable hs_,
                                const char * key ) {
    assert( key );
    assert( hs_ );
    auto hs = reinterpret_cast<HT *>(hs_);
    auto it = hs->find( key );
    if( hs->end() == it ) {
        return NULL;
    }
    return it->second;
}

void gds_hashtable_erase(       gds_Hashtable hs_,
                                const char * key ) {
    assert( key );
    assert( hs_ );
    auto hs = reinterpret_cast<HT *>(hs_);
    auto it = hs->find( key );
    if( hs->end() == it ) {
        emraise( noSuchKey,
                 "No name at hashtable %p: \"%s\".",
                 hs_, key );
    }
    hs->erase( it );
}

# endif  // ENABLE_GDS

