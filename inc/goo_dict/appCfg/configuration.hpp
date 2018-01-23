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

struct option;

namespace goo {
namespace dict {

template<>
class AppConfTraits::DictionaryAspect<char> : public std::unordered_set<AppConfTraits::VBase *> {
public:
    //void emplace
    virtual ~DictionaryAspect() {
        for( auto p : *this ) {
            delete p;
        }
    }
};

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
 * `invalidate_getopt_caches()` just before `extract()` invokation if inserted
 * sub-sections were modified.
 * */
class Configuration : public mixins::iDuplicable< AppConfTraits::VBase
                                              , Configuration
                                              , AppConfNameIndex> {
public:
    typedef mixins::iDuplicable< AppConfTraits::VBase
                               , Configuration
                               , AppConfNameIndex
                               > DuplicableParent;
    typedef GenericDictionary< char
                             , aspects::Description
                             , aspects::iStringConvertible
                             , aspects::CharShortcut
                             , aspects::Required
                             , aspects::IsSet
                             , aspects::Array> ShortcutsIndex;
    /// The logic parameter may be defined as option without a value (flag)
    /// according to POSIX convention, thus this type has to be treated
    /// differently. This important case is checked with expensive
    /// dynamic_cast<>.
    typedef Parameter< bool
                     , aspects::Description
                     , aspects::iStringConvertible
                     , aspects::CharShortcut
                     , aspects::Required
                     , aspects::IsSet
                     , aspects::Array> LogicOption;

    typedef AppConfTraits::VBase VBase;
private:
    /// Dictionary for shortcut-only parameters. This container is bound with
    /// Configuration instance meaning that its entries lifetime is restricted
    /// by Configuration instance lifetime.
    ShortcutsIndex _shortcutsIndex;
    /// Positional parameter. Ptr may be null if positional argument is
    /// disallowed.
    std::pair<std::string, VBase *> _positionalArgument;
public:
    /// Ctr expects the `name' here to be an application name and `description'
    /// to be an application description.
    /// @param defaultHelpIFace controls, whether the -h/--help/-? flags will
    ///        be automatically inserted.
    explicit Configuration( const char * description );
    ~Configuration();

    /// Copy ctr.
    Configuration( const Configuration & );

    /// Explicit copy creation.
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

    /// Returns forwarded arguments (if they were set).
    const Array<std::string> & forwarded_argv() const;

    bool is_consistent( Hash<std::string, const VBase *> & ) const;
};  // class Configuration

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_CONFIGURATION_H

