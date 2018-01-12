# ifndef H_GOO_DICT_LOS_H
# define H_GOO_DICT_LOS_H

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

# include "goo_dict/parameter.tcc"
# include "goo_dict/parameters/pointer_parameter.tcc"

namespace goo {
namespace dict {

class ObjectsListParameter : public iTValue<List<iStringConvertibleParameter *> > {
protected:
    virtual void _set_value( const Value & ) override;
public:
    typedef List<iStringConvertibleParameter *> Value;
    // ...
    /// Const value getter (public). Raises "unitialized" error if value was
    /// not set.
    virtual const Value & value() const;
};

template<>
class iParameter<List<iStringConvertibleParameter *> > : public iAbstractParameter
                                                       , public ObjectsListParameter {
public:
    //typedef iTStringConvertibleParameter<List<iStringConvertibleParameter *> > ValueStoringType;
    typedef List<iStringConvertibleParameter *> Value;
    typedef iSingularParameter::ParameterEntryFlag ParameterEntryFlag;
public:
    iParameter( const char * name,
                const char * description,
                ParameterEntryFlag,
                char shortcut_ = '\0' );

    iParameter( const iParameter<Value> & );

    ~iParameter() {}
};  // class iParameter

template<>
class Parameter< List<iStringConvertibleParameter*> > :
                public mixins::iDuplicable< iAbstractParameter
                                          , Parameter< List<iStringConvertibleParameter*> >
                                          , iParameter< List<iStringConvertibleParameter*> >
                                          > {
public:
    typedef List<iStringConvertibleParameter*> Array;
    typedef mixins::iDuplicable< iAbstractParameter
                               , Parameter< List<iStringConvertibleParameter*> >
                               , iParameter< List<iStringConvertibleParameter*> >
                               > DuplicableParent;
public:
    Parameter( const Parameter & );
    Parameter( const char *
             , const char * );
};  // class LoS

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_DICT_LOS_H
