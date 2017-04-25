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

# include "goo_dict/conf_help_render.hpp"

# include "goo_dict/configuration.hpp"

# include <algorithm>

namespace goo {
namespace dict {

void
POSIXRenderer::_V_render_help_page( std::ostream & os,
                                 const char * appName ) {

    os << conf().name() << ": " << conf().description() << std::endl
       << "Usage:" << std::endl
       << "    $ ";
    // Print utility name:
    if( appName ) {
        os << appName;
    } else {
        os << conf().name();
    }
    os << " ";

    _recollect_first_level_options( conf(), "" );

    if( !_shrtFlags.empty() ) {
        std::string shrtFlags;
        std::transform( _shrtFlags.begin(), _shrtFlags.end(),
                std::back_inserter(shrtFlags),
                std::bind( &DECLTYPE(_shrtFlags)::value_type::first,std::placeholders::_1 ) );
        std::sort( shrtFlags.begin(), shrtFlags.end());
        os << "[-" << shrtFlags << "] ";
    }

    {
        for( const auto & p : _shrtFlags ) {
            assert( !p.second->name() );
            if( p.second->requires_value() ) {
                os << _singular_parameter_usage_info(*(p.second) ) << " ";
            }
        }
    }

    {
        for( const auto & p : _requiredNamed ) {
            os << _singular_parameter_usage_info(
                                    *(p.second), p.first.c_str() ) << " ";
        }
    }

    if( ! conf().dictionaries().empty() ) {
        os << "[" ESC_CLRUNDRLN "SECT OPTIONS" ESC_CLRCLEAR "]";
    }

    // TODO os << _positionalPtr

    os  << std::endl << std::endl
        << "Where:" << std::endl;

    render_reference( os, conf() );

    if( !conf().dictionaries().empty() ) {
        os << "The " ESC_CLRUNDRLN "SECT OPTIONS" ESC_CLRCLEAR " placeholder "
           "refers to parameters in sub-sections. Type " ESC_CLRBOLD
           "--help=<subsection-name>" ESC_CLRCLEAR " or " ESC_CLRBOLD "-h<subsection-name>" ESC_CLRCLEAR
           " to get help about parameters from sub-sections."
           << std::endl
           ;
    }
}

void
POSIXRenderer::_V_render_reference( std::ostream & os,
                          const Dictionary & d ) {
    std::map<std::string, const iSingularParameter *> mySortedPs;
    const auto & myParameters = d.parameters();
    std::transform( myParameters.begin(), myParameters.end(),
                std::inserter( mySortedPs, mySortedPs.end() ),
                //std::bind( &DECLTYPE(myParameters)::value_type, std::placeholders::_1 )
                []( iSingularParameter * pPtr )
                    { return std::make_pair( (pPtr->name() ? pPtr->name()
                                                           : std::string(1, pPtr->shortcut())),
                                              pPtr ); } );
    for( auto p : mySortedPs ) {
        os << "  " << _singular_parameter_usage_info( *(p.second), p.first.c_str() )
           << /*TODO: more verbose info about flags, type, default, etc*/ std::endl
           << "        " << p.second->description()
           << std::endl
           ;
    }
    if( !d.dictionaries().empty() ) {
        // TODO: sort?
        os  << "There are also " << d.dictionaries().size()
            << " sub-sections available in this section:" << std::endl;
        for( auto p : d.dictionaries() ) {
            os << "    " ESC_CLRBOLD << p.first << ESC_CLRCLEAR "" << std::endl;
        }
    }
}

std::string
POSIXRenderer::_singular_parameter_usage_info(
                                    const iSingularParameter & p,
                                    const char * lnName ) {
    char nameID[128],
         valueID[128],
         fullID[256]
         ;

    if( p.has_shortcut() ) {
        if( !lnName ) {
            // Shortcut-only.
            snprintf( nameID, sizeof(nameID),
                "-" ESC_CLRBOLD "%c" ESC_CLRCLEAR, p.shortcut() );
        } else {
            // Fully-qualified name.
            snprintf( nameID, sizeof(nameID),
                "-" ESC_CLRBOLD "%c" ESC_CLRCLEAR "|--" ESC_CLRBOLD "%s" ESC_CLRCLEAR, p.shortcut(), lnName );
        }
    } else if( lnName ) {
        // Long name-only.
        snprintf( nameID, sizeof(nameID), "--" ESC_CLRBOLD "%s" ESC_CLRCLEAR, lnName );
    }
    // TODO: else --- positional
    
    if( p.requires_value() ) {
        if( !p.is_set() ) {
            // Has no default value:
            snprintf( valueID, sizeof(valueID), " " ESC_CLRUNDRLN "%s" ESC_CLRCLEAR,
                      p.target_type_info().name() );
        } else {
            // Has default value:
            snprintf( valueID, sizeof(valueID), " " ESC_CLRUNDRLN "%s" ESC_CLRCLEAR "(=default)",
                      p.target_type_info().name() );
        }
    } else {
        valueID[0] = '\0';
    }

    if( p.is_optional() || p.is_set() ) {
        // May be omitted:
        if( !p.has_multiple_values() ) {
            snprintf( fullID, sizeof(fullID), "[%s%s]", nameID, valueID );
        } else {
            snprintf( fullID, sizeof(fullID), "[%s%s]...", nameID, valueID );
        }
    } else {
        if( !p.has_multiple_values() ) {
            snprintf( fullID, sizeof(fullID), "%s%s", nameID, valueID );
        } else {
            snprintf( fullID, sizeof(fullID), "%s%s...", nameID, valueID );
        }
    }
    return fullID;
}

void
POSIXRenderer::_recollect_first_level_options(
                const Dictionary & self,
                const std::string & nameprefix ) {
    // Iterate among dictionary options collecting the parameters:
    for( auto it  = self.parameters().begin();
              it != self.parameters().end(); ++it ) {
        // Collect, if parameter has the shortcut or is mandatory:
        if( (*it)->has_shortcut() || (*it)->is_mandatory() ) {
            if( (*it)->name() ) {
                // parameter has long name, collect it with its full path:
                # ifndef NDEBUG
                auto ir = _requiredNamed.emplace( nameprefix + (*it)->name(), *it );
                assert( ir.second );
                # else
                _requiredNamed.emplace( nameprefix + (*it)->name(), *it );
                # endif
            } else {
                // parameter has no long name, collect it with its shortcut:
                # ifndef NDEBUG
                auto ir = _shrtFlags.emplace( (*it)->shortcut(), *it );
                assert( ir.second );
                # else
                _shrtFlags.emplace( (*it)->shortcut(), *it );
                # endif
            }
            // TODO: positional?
        }
    }
    for( auto it  = self.dictionaries().cbegin();
              it != self.dictionaries().cend(); ++it ) {
        std::string sectPrefix = nameprefix
                               + it->second->name()
                               + ".";
        _recollect_first_level_options( *(it->second), sectPrefix );
    }
}

}  // namespace dict
}  // namespace goo

