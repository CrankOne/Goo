# include "goo_config.h"

# ifdef ENABLE_GDS

# include <unordered_map>
# include <unordered_set>
# include <cassert>

# include "gds/goo_interpreter.h"
# include "goo_exception.hpp"

//
// Hashtable
//

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

//
// Heap
//

typedef std::unordered_set<void *> Heap;

gds_Heap
gds_heap_new() {
    return new Heap();
}

void
gds_heap_free( gds_Heap hp_ ) {
    assert( hp_ );
    Heap * hp = reinterpret_cast<Heap *>(hp_);
    delete hp;
}

void *
gds_heap_alloc( gds_Heap hp_, uint32_t blocklen ) {
    assert( hp_ );
    Heap * hp = reinterpret_cast<Heap *>(hp_);
    void * chunk = malloc( blocklen );
    hp->insert( chunk );
    return chunk;
}

void
gds_heap_erase( gds_Heap hp_, void * chunk ) {
    assert( hp_ );
    Heap * hp = reinterpret_cast<Heap *>(hp_);
    # ifndef NDEBUG
    auto it = hp->find( chunk );
    if( hp->end() == it ) {
        emraise( noSuchKey, "Block %p isn't in the heap %p.", chunk, hp_ );
    }
    # endif
    free( *it );
}

# endif  // ENABLE_GDS

