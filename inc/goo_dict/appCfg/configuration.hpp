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

# ifndef H_GOO_PARAMETERS_CONFIGURATION_H
# define H_GOO_PARAMETERS_CONFIGURATION_H

# include "goo_dict/appCfg/traits.hpp"
# include "insertion_proxy.tcc"
# include "goo_dict/util/dpath.hpp"

# include <alloca.h>


struct option;

namespace goo {
namespace utils {
struct getopt_ConfDictCache;
}
namespace dict {

//}  // namespace dict

//namespace app {

/**@brief Class representing dictionaries root instance.
 * @class Configuration
 *
 * This class designed mostly for maintaining the application configuration
 * with strong emphasis on command-line arguments parsing. The native system's
 * `getopt_long()` function will perform actual parsing in `extract()` method
 * providing GNU-extended POSIX compatibility of command-line arguments
 * syntax.
 *
 * Warning: subsections have no back-populate effect, so modifying them after
 * they were added to Configuration instance will not cause chaches to be
 * invalidated. Thus, it is mandatory to manually invalidate caches with
 * `invalidate_getopt_caches()` just before `extract()` invocation if inserted
 * sub-sections were modified.
 * */
class Configuration : public mixins::iDuplicable< typename AppConfTraits::template IndexBy<std::string>::DictValue::Base
                                                , Configuration
                                                , AppConfNameIndex> {
    friend struct ::goo::utils::getopt_ConfDictCache;
    friend class ::goo::dict::InsertionProxy<std::string>;
public:
    typedef mixins::iDuplicable< typename AppConfTraits::template IndexBy<std::string>::DictValue::Base
                               , Configuration
                               , AppConfNameIndex> DuplicableParent;
    typedef GenericDictionary< char, _Goo_m_VART_LIST_APP_CONF > ShortcutsIndex;
    /// The logic parameter may be defined as option without a value (flag)
    /// according to POSIX convention, thus this type has to be treated
    /// differently. This important case is checked with expensive
    /// dynamic_cast<>. TODO: implicit arguments?
    typedef Parameter< bool, _Goo_m_VART_LIST_APP_CONF > LogicOption;

    typedef AppConfTraits::FeaturedBase FeaturedBase;
private:
    /// Dictionary for shortcut-only parameters. This container is bound with
    /// Configuration instance meaning that its entries lifetime is restricted
    /// by Configuration instance lifetime.
    ShortcutsIndex _shortcutsIndex;
    /// Positional parameter. Ptr may be null if positional argument is
    /// disallowed.
    std::pair<std::string, FeaturedBase *> _positionalArgument;
protected:
    void _add_shortcut( char, FeaturedBase * p );
    FeaturedBase * positional_argument_ptr() { return _positionalArgument.second; }
    const FeaturedBase * positional_argument_ptr() const { return _positionalArgument.second; }
public:
    /// Ctr expects the `name' here to be an application name and `description'
    /// to be an application description.
    /// @param defaultHelpIFace controls, whether the -h/--help/-? flags will
    ///        be automatically inserted.
    explicit Configuration( const std::string & description );
    ~Configuration();

    /// Copy ctr.
    Configuration( const Configuration & );

    virtual InsertionProxy<std::string> insertion_proxy() override {
        return InsertionProxy<std::string>( this );
    }

    const ShortcutsIndex & short_opts() const {
        return _shortcutsIndex;
    }

    // Explicit copy creation.
    //Configuration copy() const { return *this; }

    /// Makes configuration able to acquire single (only) positional argument.
    /// This is not an incremental procedure and has to be used rarely: if
    /// user's code has to treat multiple positional arguments, the
    /// `positional_arguments()` has to be used instead.
    template<typename T> void
    positional( const char name[], const char description[] ) {
        _TODO_  // todo: as InsertionProxy does.
        # if 0
        auto * p = new InsertableParameter<T>( name, description );
        p->_check_initial_validity();
        _positionalArgument = p;
        # endif
    }

    /// Makes configuration instace be able to acquire a list of positional
    /// arguments of the specific type.
    template<typename T> void
    positional_arguments( const char name[], const char description[] ) {
        _TODO_  // todo: as InsertionProxy does.
        # if 0
        auto p = new InsertableParameter<Array<T> >( name, description );
        _positionalArgument = p;
        # endif
    }

    /// Overrides the default entry getter to support positional argument and the
    /// single character names usually referring to shortcuts (const ver).
    virtual const FeaturedBase * entry_ptr( const std::string & name ) const override {
        assert( !name.empty() );
        if( _positionalArgument.second
         && _positionalArgument.first.size()
         && _positionalArgument.first == name ) {
            return _positionalArgument.second;
        }
        if( 1 == name.size() ) {
            auto it = _shortcutsIndex.value().find( name[0] );
            if( _shortcutsIndex.value().end() != it ) {
                return it->second;
            }
        }
        return DuplicableParent::entry_ptr( name );
    }

    /// Overrides the default entry getter to support positional argument and the
    /// single character names usually referring to shortcuts (mutable ver).
    virtual FeaturedBase * entry_ptr( const std::string & name ) override {
        assert( !name.empty() );
        if( _positionalArgument.second
         && _positionalArgument.first.size()
         && _positionalArgument.first == name ) {
            return _positionalArgument.second;
        }
        if( 1 == name.size() ) {
            auto it = _shortcutsIndex.value().find( name[0] );
            if( _shortcutsIndex.value().end() != it ) {
                return it->second;
            }
        }
        return DuplicableParent::entry_ptr( name );
    }

    /*
    const FeaturedBase & operator[](char c) const {
        auto it = _shortcutsIndex.value().find(c);
        if( _shortcutsIndex.value().end() == it ) {
            emraise( notFound
                   , "Configuration %p has no option with shortcut '%c'."
                   , this, c );
        }
    }
     */

    # if 0  // does not seems that we really need non-const getter here.
    FeaturedBase & operator[](char c) {
        auto it = _shortcutsIndex._mutable_value().find(c);
        if( _shortcutsIndex.value().end() == it ) {
            emraise( notFound
                   , "Configuration %p has no option with shortcut '%c'."
                   , this, c );
        }
    }
    # endif

    /// Returns forwarded arguments (if they were set).
    const Array<std::string> & forwarded_argv() const;

    bool is_consistent( Hash<std::string, const FeaturedBase *> & ) const;
};  // class Configuration

}  // namespace dict

namespace utils {

# if 0
struct IConfDictCache {

    void cache_long_options( const std::string & nameprefix
                           , const dict::AppConfNameIndex & d
                           , IConfDictCache & self );

    /// Will be called by for_all_options() for each entry.
    virtual void consider_entry( const std::string & name
                               , const std::string & nameprefix
                               , dict::AppConfTraits::FeaturedBase & ) = 0;

    // dict::AppConfNameIndex::RecursiveVisitor
};
# endif

struct getopt_ConfDictCache {
public:
    /// This two arrays keep the common prefixes for getopt() shortcuts string.
    static const char defaultPrefix[8];
    virtual void consider_entry( const std::string & name
                               , const std::string & nameprefix
                               , dict::AppConfTraits::FeaturedBase & );
private:
    bool _dftHelpIFace;
    std::string _shorts;
    std::vector<struct ::option> _longs;
    std::unordered_map<int, dict::Configuration::FeaturedBase *> _lRefs;
    dict::Configuration::FeaturedBase * _posArgPtr;
    /// This variable where option identifiers will be loaded into.
    int _longOptKey;
    /// Temporary data attribute, active only during the recursive traversal.
    mutable std::string _nameprefix;
public:
    explicit getopt_ConfDictCache( dict::Configuration & cfg
                                 , bool dftHelpIFace=true );

    const std::string & shorts() const { return _shorts; }
    const std::vector<struct ::option> & longs() const { return _longs; };
    bool default_help_interface() const { return _dftHelpIFace; }
    dict::Configuration::FeaturedBase * positional_arg_ptr() const { return _posArgPtr; }
    dict::Configuration::FeaturedBase * current_long_parameter( int );

    /// Used to fill caches during recursive traversal:
    void operator()( dict::Hash<std::string, dict::AppConfTraits::FeaturedBase *>::iterator );
};

/**@brief A utility function performing initialization of an existing
 *        goo::dict::Configuration instance with given argc/argv expression.
 *
 * This function is a main procedure to initialize application configuration
 * represented as an instance of goo::dict::Configuration with standard C/C++
 * argc/argv data, usually provided to application by standard system invocation
 * mechanism. By the consistency reasons, we've decided to use system POSIX
 * getopt_long() function to perform option interpretation, so most of the
 * standard POSIX command line options features are supported.
 *
 * @param cfg Configuration instance.
 * @param argc number of argv tokens (>0)
 * @param argv array of strings (option tokens)
 * @param doCnstCheck wether to perform internal the consistency check for
 *        completeness
 * @param cachePtr Configuration-to-getopt_long()-struct cache instance ptr. Set
 *        it to nullptr to make routine allocate temporary one.
 * @param logStreamPtr ptr to output logging stream where internal message will
 *        be printed
 **/
int set_app_conf( dict::Configuration & cfg
                , int argc
                , char * const * argv
                , getopt_ConfDictCache * cachePtr=nullptr
                , std::ostream *logStreamPtr=nullptr );  // TODO: migrate impl from dev branch

std::string compose_reference_text_for( const dict::Configuration & cfg
                                    , const std::string & sect="" );

void set_app_cfg_parameter( dict::Configuration::FeaturedBase & v
                          , const char * strExpr
                          , std::ostream * verbose );

}  // namespace utils
}  // namespace goo

# endif  // H_GOO_PARAMETERS_CONFIGURATION_H

