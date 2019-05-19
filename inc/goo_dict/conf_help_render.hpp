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

# ifndef H_GOO_CONFIGURATION_HELP_RENDERED_H
# define H_GOO_CONFIGURATION_HELP_RENDERED_H

# include "goo_exception.hpp"

# include <cassert>
# include <map>
# include <set>

namespace goo {
namespace dict {

class Configuration;
class Dictionary;
class iSingularParameter;

/**@class iHelpRenderer
 * @brief Aux class defining how to generate help reference text from
 *        application configuration dictionary.
 * */
class iHelpRenderer {
private:
    const Configuration * _confPtr;
protected:
    virtual void _V_render_help_page( std::ostream &,
                                      const char * appName=nullptr ) = 0;
    virtual void _V_render_reference( std::ostream &,
                                      const Dictionary & dict,
                                      const std::string & prefix ) = 0;
public:
    iHelpRenderer( const Configuration & c ) : _confPtr(&c) {}

    iHelpRenderer() : _confPtr(nullptr) {}

    /// Renders fully-qualified help message for given configuration.
    void render_help_page( std::ostream & os,
                           const char * appName=nullptr ) {
        _V_render_help_page( os, appName );
    }

    /// Renders help message for particular section.
    void render_reference(  std::ostream & os,
                            const Dictionary & dict,
                            const std::string prefix="") {
        assert( _confPtr );
        _V_render_reference(os, dict, prefix);
    }

    /// Returns
    const Configuration & conf() const {
        if( !_confPtr ) {
            emraise( badState, "Configuration instance has to be associated "
                "with help page renderer (but it is not)." );
        }
        return *_confPtr;
    }
};

class POSIXRenderer : public iHelpRenderer {
protected:
    /// Short-only logic flags.
    std::map<char, const iSingularParameter *> _shrtFlags;
    std::map<std::string, const iSingularParameter *> _requiredNamed;
    iSingularParameter * _positionalPtr;

protected:
    /// Performs initial traversal among all the subsections to collect
    /// shortened options, required arguments and positional arguments that
    /// must be printed at top level reference page.
    virtual void _recollect_first_level_options(
                        const Dictionary &,
                        const std::string &,
                        bool thisIsBaseLevel=false);

    /// Returns string describing parameter usage information with its
    /// fully-qualified name.
    virtual std::string _singular_parameter_usage_info(
                        const iSingularParameter &,
                        const char * fullname=nullptr );

    virtual void _V_render_help_page( std::ostream &,
                                      const char * appName=nullptr ) override;

    void _V_render_reference( std::ostream &,
                              const Dictionary &,
                              const std::string &) override;

public:
    POSIXRenderer( const Configuration & c ) : iHelpRenderer(c) {}
};  // class HelpRenderer

/**@class LineListRenderer
 * @brief A simple configuration rendering class useful for bash completion.
 *
 * This help renderer was designed for bash completion util. It produces
 * merely a list of options delimeted with spaces for them to further be used
 * with bash completion facility.
 *
 * TODO: enum
 * */
//class LineListRenderer;

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_CONFIGURATION_HELP_RENDERED_H

