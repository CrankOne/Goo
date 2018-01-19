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

#ifndef H_GOO_PARAMETERS_APP_CONF_INFO_H
#define H_GOO_PARAMETERS_APP_CONF_INFO_H

# include "goo_types.h"

namespace goo {
namespace dict {
namespace pInfos {

struct DescriptionInfo {
    // ...
};

struct RequiredParameterInfo {
    // ...
};

struct IsSetInfo {
    // ...
};

}  // namespace pInfos

/**@class AppConfParameter
 * @brief An abstract parameter is a base class for Goo's
 * dictionary entires.
 *
 * We're tending to follow here to POSIX convention:
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html#tag_12_02
 * extended with GNU long options:
 * https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html ,
 *
 * so the option-arguments are never optional
 * (see Guideline 7 of 12.2). That means one can not, for example, declare
 * an option `-c` with optional argument. Even if the `-c` option is a
 * logical flag the option-argument should either be necessarily presented,
 * or prohibited by declaration. In first case this option have to be
 * provided either as `-c true` or `-c false` (or in any equivalent
 * appropriate way for options of logic type). In the second case this option
 * can not receive an option-argument (and should be provided as `-c` or
 * omitted).
 *
 * The following subclasses are expected:
 *  - single-char keys (here referred as `shortcuts')   (flags or with values, multiple time)
 *  - long options                                      (flags or with values)
 *  - unnamed positional arguments
 *  - sections (dictionaries itself)
 *
 * Flag can not be required (required=false)
 *
 * Required can not be set (set=false)
 *
 * List-of-Structures has atomic=false, singular=true (TODO!)
 *
 * @ingroup appParameters
 */
class AppConfParameter {
    # if 0
 public:
    typedef UByte ParameterEntryFlag;
    static const ParameterEntryFlag
            set,            ///< Value is set (=false on init for required/flag/dict).
            flag,           ///< Two-state logical parameter without an argument.
            positional,     ///< Positional argument (=false for flag/dict).
            atomic,         ///< Indicates true parameter (otherwise, it's a dictionary).
            singular,       ///< Can be provided only once (=false for dict).
            required,       ///< A mandatory parameter entry (=false for flag/dict).
            shortened       ///< Has a shortcut (single-letter option, =false for positional).
    ;
private:
    char * _name           ///< Name of the option. Can be set to nullptr.
       , * _description;    ///< Description of the option. Can be set to nullptr.
    /// Stores logical description of an instance.
    ParameterEntryFlag _flags;
protected:
    /// Used only when shortened flag is set.
    char _shortcut;
protected:
    /// Sets the "set" flag translating instance to initialized state.
    void _set_is_set_flag();

    /// Sets the "is flag" flag.
    void _set_is_flag_flag();

    /// This method is to be used by lists only.
    void _unset_singular();

    /// Single ctr can be only invoked by descendants.
    AppConfParameter( const char * name,
            const char * description,
            ParameterEntryFlag flags,
            char shortcut = '\0');

    /// Copy ctr.
    AppConfParameter( const AppConfParameter & );

    /// Raises an exception if contradictory states are set for parameter
    /// on initial stage.
    void _check_initial_validity();

    /// Useful for auxilliary classes.
    void _append_description( const char * );
public:
    virtual ~AbstractParameter();

    /// Returns pointer to name string.
    const char * name() const;

    /// Name setter. Use with care since these parameters may often be cached
    /// by owning dictionaries.
    void name( const char * );

    /// Returns pointer to description string.
    const char * description() const;

    /// Returns shortcut if it was set. Otherwise returns '\0'.
    char shortcut() const { return _shortcut; }

    /// Returns true, if parameter has a value set (even if it is a default one).
    bool is_set() const {
        return _flags & set;
    }
    /// Returns true, if parameter means logical flag.
    bool is_flag() const {
        return _flags & flag;
    }
    /// Returns true, if parameter expects a value (or already contain a default one).
    bool requires_value() const {
        return !is_flag();
    }
    /// Returns true, if parameter has no name (at all --- even one-letter shortcut).
    bool is_positional() const {
        return _flags & positional;
    }
    /// Returns true, if parameter is not a dictionary.
    bool is_atomic() const {
        return _flags & atomic;
    }
    /// Returns true, if parameter is a dictionary.
    bool is_dictionary() const {
        return !is_atomic();
    }
    /// Returns true, if parameter has only one value.
    bool is_singular() const {
        return _flags & singular;
    }
    /// Returns true, if parameter has a set of values.
    bool has_multiple_values() const {
        return !is_singular();
    }
    /// Returns true, if parameter is mandatory.
    bool is_mandatory() const {
        return _flags & required;
    }
    /// Returns true, if parameter can be omitted.
    bool is_optional() const {
        return !is_mandatory();
    }
    /// Returns true, if parameter has a single-character shortcut.
    bool has_shortcut() const {
        return _flags & shortened;
    }

    /// Sets the "required" flag marking a mandatory parameter.
    void set_is_argument_required_flag();
    # endif
};  // class AppConfParameter

}  // namespace dict
}  // namespace goo

#endif  // H_GOO_PARAMETERS_APP_CONF_INFO_H
