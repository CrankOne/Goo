# include <cstring>
#include <goo_dict/types.hpp>

# include "goo_dict/insertion_proxy.tcc"
# include "goo_dict/dict.hpp"

# include "goo_dict/parameters/los.hpp"

namespace goo {
namespace dict {


# if 0
/**This routine serves few use cases, but basically it is designed to perform
 * recursive evaluation according to textual path specified, within given
 * goo::pdict data structure. Few major use-cases are:
 *
 * 1. Non-invasive look-up. Any not found entry will cause goo::notFound
 * exception.
 * 2. Extension. Routine will try to insert new subsections and lists inside
 * the starting entity.
 *
 * Path examples:
 *  - "one.#23" --- for non-invasive look-up this path expression will retrieve
 *  iBaseValue pointer to 23-rd element of list "one" in
 *  starting dictionary. In extending mode it will add 23 element to list "one"
 *  in the starting dictionary. If list "one" does not exist or contains less
 *  than 22 elements, even extending look-up mode will emit the exception
 *  (goo::overflow in both cases).
 *  - "#11.#0" --- for non-invasive look-up, will retrieve first element of
 *  element 11 of starting list.
 *  - "one.#" --- possible only in extending mode. Will cause routine to insert
 *  the list named "one" at the starting dictionary.
 *
 *  TODO: test this thing. Very. Carefully.
 *
 *  @param currentStack describes the target stack instance where references to
 *      dictionaries or lists have to be put in.
 *  @param path_ is a textual path.
 *  @param extend controls whether to insert new lists or dictionaries. Being
 *      set to `false` causes routine to emit notFound exception.
 * */
InsertionProxyBase::MaterializedPath
InsertionProxyBase::combine_path( InsertionTargetsStack & mpath
                                , aux::DictPath & path
                                , bool extend
                                , const std::string ed ) {
    // This macro defines finishing block. Depending on whether the last token
    // was extracted from path, the routine will terminate itself, or invoke
    // itself further.
    # define CONTINUE_RECURSIVELY                               \
    if( path.next ) {                                           \
        return combine_path( mpath, *(path.next), extend, ed );  \
    } else { return mpath; }

    const bool /*emptyToken = path.isIndex ? 0 == path.id.index : !strlen( path.id.name )
             ,*/ lastToken = !( path.next )
             , isIndex = !(path.isIndex)
             ;

    //// Routine allows empty token to be the the only last one index identifier.
    //if( emptyToken && (!extend || !lastToken) ) {
    //    emraise( badParameter, "Empty token met in path specification within"
    //            " non-extending call." );
    //}  // TODO: ^^^

    if( lastToken ) {
        // Last token probably refers to a parameter.
        iSingularParameter * isp \
                = DictInsertionAttorney::probe<iSingularParameter>( path.id.name
                                                        , mpath.top().as<Dict>() );
        if( isp ) {
            return MaterializedPath( mpath, isp );
        }
        // Otherwise we have to consider this last token as one referencing to
        // a dictionary or a list instance.
    }
    if( !isIndex ) {
        if( lastToken || (!lastToken && !(path.next->isIndex) ) ) {
            Dict * newStart = DictInsertionAttorney::probe<DictionaryParameter>(
                            path.id.name, mpath.top().as<Dict>() );
            if( newStart ) {
                // That's a dict indeed. Continue recursively.
                mpath.push( newStart );
                CONTINUE_RECURSIVELY;
            } else if( !lastToken ) {
                // TODO: dedicated notFound subclass for dict look-up procedure
                emraise( notFound, "No subsection named \"%s\".", path.id.name );
            }
        }

    } else {
        _TODO_
        //LoS * newStart = ListInsertionAttorney::probe_list( path.id.index
        //                                                  , mpath.top().as<LoS>() );
        //
    }

    # if 0
    if( !isIndex ) {
        Dict & startDctRef = mpath.top().as<Dict>();
        // Token is a string. Consider start as (at least) of type Dict
        // and try to retrieve parameter or subsection named as `current'
        // refers.
        {

            Dict * newStart = DictInsertionAttorney::probe_subsection(
                        path.id.name, startDctRef);
            if( newStart ) {
                // That's a dict indeed. Continue recursively.
                mpath.push( newStart );
                CONTINUE_RECURSIVELY;
            }
        }
        {
            LoS * newStart = DictInsertionAttorney::probe_list(current, startDctRef);
        }
        // Check whether token refers to parameter
        {
            iSingularParameter * pPtr = DictInsertionAttorney::probe_parameter( current );
            if( pPtr ) {
                // Parameter found. Try to downcast it to dict and consider
                // next token in a path as an index.
                auto newStart = dynamic_cast<NamedLoS*>( pPtr );
                if( !newStart ) {
                    if( 0x2 & rc ) {
                        // The parameter must be a subclass of LoS in this
                        // context.
                        emraise( notFound, "Interim path token \"%s\" refers to"
                                " existing singular parameter %p within a"
                                " dictionary, but this parameter does not"
                                " support subsequent indexing.", current, pPtr );
                    } else {
                        // Token terminating path refers to a parameter.
                        return MaterializedPath( mpath, pPtr );
                    }
                }
                mpath.push( newStart );
                CONTINUE_RECURSIVELY;
            }
        }
        // Following code corresponds to the case of not found entry within a
        // dictionary
        if( !extend ) {
            std::string nm = start.get_name("<anonymous>");
            emraise( notFound, "No entry \"%s\" found within dictionary"
                   " %p (within %s dict)."
                   , current, &startDctRef, nm.c_str() );
        }
        // To extend a dictionary within given path, we have to assume the
        // current parameter type. It may be either a named dict
        // (subsection), or a list here. The obvious way is to look for the
        // next token in a string path --- whether it starts from # sign.
        if( '#' != *path ) {
            // It is a named dict.
            auto newStart = new NamedDict( current, ed.c_str() );
            startDctRef.insert_section( newStart );
            mpath.push( newStart );
            return combine_path( mpath, path, extend, ed.c_str() );
        } else {
            // It is a named LoS.
            //auto newStart = new NamedLoS( current, ed.c_str() );
            auto newStart = new NamedLoS(current, ed.c_str());
            startDctRef.insert_parameter( newStart );
            mpath.push( static_cast<NamedLoS*>(newStart) );
            return combine_path( mpath, path, extend, ed.c_str() );
        }
        // NOTE: one has to check, whether the situations like "one.two.#" or
        // "one.two." works correctly.
    } else {
        // Token is an integer (index). Consider start as (at least of type
        // LoS and try to retrieve element referenced by this index.
        LoS & startLoSRef = start.as<LoS>(false);
        bool newElement = false;
        if( index >= (long) startLoSRef.value().size() ) {
            if( !extend ) {
                emraise( notFound, "Array object %p (%s) has only %zu elements"
                        ", but #%ld-th element is requested."
                         , &startLoSRef
                         , start.get_name( "<anonymous>" ).c_str()
                         , startLoSRef.value().size()
                         , index );
            }
            if( index != (long) startLoSRef.value().size() ) {
                newElement = true;
            } else {
                emraise( overflow, "Unable to extend list %p (%s) currently"
                       " having %zu elements with %ld-th element (number"
                       " has to refer the last element in a list)."
                       , &startLoSRef
                       , start.get_name( "<anonymous>" ).c_str()
                       , startLoSRef.value().size()
                       , index );
            }
        }
        // Before we start to handle the element list as a materialized
        // path entry, we have to find out its particular type in terms
        // of goo::dict containers --- whether or not it is a list or a
        // dict. Since we're currently operating within list context both
        // of them have to be anonymous.

        //iBaseValue * scpEl;
        /*
        iBaseValue * scpEl = startLoSRef.value().at(index);
        // Since path implies, that retrieved element is not the last one,
        // try to cast
        bool isList = '#' == *path;
        if( isList ) {
            LoS * losPtr = dynamic_cast<LoS*>(scpEl);
            if( losPtr ) {
                NamedLoS * nLoSPtr = dynamic_cast<NamedLoS*>(scpEl);
                mpath.push( nLoSPtr );
                return combine_path( mpath, path, extend, ed );
            }
            //mpath
            //return ;
        }
         */
    }
    # endif

    _TODO_

    # if 0
    while(0x2 & (rc = DictionaryParameter::pull_opt_path_token( path
                                                              , current
                                                              , index ))) {
        if( !(rc & 0x1) ) {
            // Extracted token refers to a named parameter or dictionary.
            NamedDict * newStart = start.as<Dict>(false).probe_subsection(current);
            if( !newStart ) {
                // Extracted token refers to non-existing dict.
                if( !extend ) {
                    emraise( notFound, "No subsection named \"%s\" found in"
                            " subsection or dictionary instance %p."
                           , current, &(h._top_as<Dict>(false)) );
                }
                // otherwise, extend
                newTop = new DictionaryParameter( current, ed.c_str() );
                // We invoke the _top_as<> with "Dict" type because current
                // routine may operate with anonymous dict at this level
                // (however, the dict we insert is not anonymous one).
                h._top_as<Dict>(false).insert_section( newTop );
            }
            h._push_dict( newTop );
        } else {
            // Extracted token refers to a parameter with given index.
            _TODO_
        }
    }
    # endif

    _TODO_
    # if 0
    if( !(newTop = _top_as<NamedDict>(true).probe_subsection( current )) ) {
        // Insert new section.
        newTop = new DictionaryParameter( current, descr );
        _top_as<NamedDict>(true).insert_section( newTop );
        _push_dict( newTop );
    } else {
        // Append existing section:
        _push_dict( newTop );
        if( !newTop->description() ) {
            newTop->_append_description( descr );
        }
    }
    return *this;
    # endif
    # undef CONTINUE_RECURSIVELY
}
# endif

//
// Dictionary Insertion Proxy
// /////////////////////////

InsertionProxy<Dictionary>::InsertionProxy( DictionaryParameter * root )
                    : Parent(root)
                    , _lastInsertedParameter( nullptr ) {
    _push_dict( root );
}

void
InsertionProxy<Dictionary>::insert_copy_of( const iSingularParameter & sp
                                  , const char * newName ) {
    _TODO_  // TODO
    # if 0
    iSingularParameter * isp =
                    clone_as<AbstractParameter, iSingularParameter>( &sp );
    if( !! newName ) {
        isp->name( newName );
    }
    _top_as<Dict>(false).insert_parameter( isp );
    # endif
}

InsertionProxy<Dictionary> &
InsertionProxy<Dictionary>::required_argument() {
    if( !_lastInsertedParameter ) {
        emraise( badState, "Insertion proxy unable to mark previously inserted"
            " argument as required one." );
    }
    _lastInsertedParameter->set_is_argument_required_flag();
    //DictInsertionAttorney::mark_last_inserted_as_required( _top_as<Dict>() );  // XXX
    return *this;
}

InsertionProxy<Dictionary> &
InsertionProxy<Dictionary>::bgn_sect( const char * name
                           , const char * descr) {
    auto newD = new DictionaryParameter( name, descr );
    _put( newD, _top_as<Dict>() );
    _push_dict( newD );
    _lastInsertedParameter = nullptr;
    return *this;
}

InsertionProxy<Dictionary> &
InsertionProxy<Dictionary>::end_sect( const char * name ) {
    DictionaryParameter * nd = dynamic_cast<DictionaryParameter *>(
            &(_top_as<Dict>()) );
    if( !nd ) {
        // This case may happen when insertion targets stack contains a valid Dict
        // pointer, but it refers not to the DictionaryParameter subclass instance.
        // Such situation may occur when user code invokes end_sect() within
        // insertion proxy previously constructed with LoS::bgn_dict() that shall
        // be closed with end_dict() instead of end_sect(). Unfortunately, we can
        // not treat it as a tolerable mistake printing warning since methods have
        // different return types.
        emraise( badState, "Dictionary composition error. end_sect(\"%s\") method"
                         " invoked instead of end_dict().",
                name ? name : "" );
    }
    _lastInsertedParameter = nullptr;
    if( name && strcmp( name, nd->name() ) ) {
        emraise( assertFailed,
                "Insertion proxy state check failed: current section is "
                "\"%s\" while \"%s\" expected.",
                nd->name(), name );
    }
    _pop();
    return *this;
}

//
// List-of-Structures Insertion Proxy
// ////////////////////////////////

InsertionProxy<Dictionary>
InsertionProxy<ListOfStructures>::end_list( const char * listName ) {
    LoSParameter * p = dynamic_cast<LoSParameter *>(&(_top_as<ListOfStructures>()));
    if( !p ) {
        emraise( badCast, "Unable to enclose the list insertion as list"
                " parameter since it is not a named entry (mismatched"
                " end_list()/end_sublist()?)." );
    }
    if( listName && strcmp(listName, p->name()) ) {
        emraise( assertFailed, "end_list() name mismatch: stack top refers to "
                "list parameter named \"%s\" while enclosing \"%s\".",
                p->name(), listName );
    }
    _pop();
    return stack();
}

InsertionProxy<Dictionary>
InsertionProxy<ListOfStructures>::bgn_dict() {
    auto d = new Dictionary();
    _put( d );
    _push_dict( d );
    return stack();
}

InsertionProxy<ListOfStructures>
InsertionProxy<ListOfStructures>::bgn_sublist() {
    auto l = new ListOfStructures();
    _put( l );
    _push_list( l );
    return stack();
}

InsertionProxy<ListOfStructures>
InsertionProxy<ListOfStructures>::end_sublist() {
    // this cast must fail normally, unless user code is not enclosing bgn_list()
    // with end_sublist().
    LoSParameter * l = dynamic_cast<LoSParameter *>(&(_top_as<ListOfStructures>()));
    if( l ) {
        emraise( badCast, "Unable to enclose the list insertion as sublist"
                " element since it is a named entry (mismatched"
                " end_list()/end_sublist()?)." );
    }
    _pop();
    return stack();
}

# if 0
DictInsertionProxy
LoDInsertionProxy::end_dict( ) {
    _TODO_
    # if 0
    _stack.pop();
    if( _stack.top().is_list() ) {
        emraise( assertFailed
               , "Insertion proxy state check failed: stack top is not a dict"
                 " while en_list(name=\"%s\") invoked."
               , listName ? listName : "<null>" );
    }
    const char * realDictName = _stack.top().dict().name();
    if( listName && !strcmp(listName, realDictName ) ) {
        emraise( assertFailed
               , "bgn_list(name=\"%s\") doesn't match to enclosing"
                 " end_list(name=\"%s\")."
               , realDictName, listName );
    }
    return _stack;
    # endif
}
# endif

}  // namespace goo
}  // namespace dicts


