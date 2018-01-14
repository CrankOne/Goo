/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_GOO_PARAMETERS_DICTIONARY_INJECTION_MAPPING_H
# define H_GOO_PARAMETERS_DICTIONARY_INJECTION_MAPPING_H

# include "goo_dict/parameter_dict.hpp"

namespace goo {
namespace dict {

/**@brief Helper class representing (and performing) mapping from one
 *        dictionary to another optionally transforming the parameter type.
 *
 * The motivation for this class comes from practical needs. It is sometimes
 * convinient to automate bunch of parameters dictionaries processing using
 * some form of declarative syntax.
 */
class DictionaryInjectionMap {
public:
    typedef bool (*Transformation)( const iSingularParameter &,
                                    iSingularParameter & );

    /// An aux class representing target (mapped) parameter.
    struct MappedEntry {
        std::string path;
        Transformation transformation;
    };  // MappedEntry

    typedef std::unordered_map<std::string, MappedEntry> Injection;
private:
    Injection _mappings;
public:
    /// Declares a single injection pair. Set transformation to null, to state
    /// type identity between mapping variables.
    std::pair<Injection::iterator, bool> add_mapping( const std::string & from,
                                                      const std::string & to,
                                                      Transformation t=nullptr);

    /// Shortcut for adding parameters mapping. Comparing to `add_mapping()`
    /// method mapped and mapping paths are inverted for convinience. Returns
    /// current instance for clearance, so that one may chain mapping in code.
    DictionaryInjectionMap & operator()(const std::string & to,
                                        const std::string & from,
                                        Transformation t=nullptr ) {
        auto ir = add_mapping( from, to, t );
        if( !ir.second ) {
            emraise( nonUniq, "Repeatative insertion of mapping \"%s\"->\"%s\".",
                from.c_str(), to.c_str() );
        }
        return *this;
    }

    /// Performs setting parameters in "target" instance with ones from "source"
    /// using paths (and transformations) previously supplied with
    /// add_mapping() method.
    void inject_parameters( const DictionaryParameter & source,
                                  DictionaryParameter & target ) const;

    /// Returns const reference of mappings for inspection.
    const Injection & injections() const { return _mappings; }
};  // class DictionaryInjectionMap

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_INJECTION_MAPPING_H

