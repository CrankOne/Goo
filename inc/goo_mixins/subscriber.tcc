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

# ifndef H_GOO_SUBSCRIBER_MIXIN_H
# define H_GOO_SUBSCRIBER_MIXIN_H

# include "goo_exception.hpp"

# include <algorithm>
# include <list>

namespace goo {
namespace mixins {


//
// Subscriber mixin
//

class IssuerMixin {
public:
    class SubscriberMixin {
    public:
        typedef IssuerMixin Issuer;
    private:
        Issuer * _issuer;
    protected:
        virtual void _V_receive() = 0;
    public:
        SubscriberMixin() : _issuer(nullptr) {}
        SubscriberMixin( Issuer & iss ) : _issuer(nullptr) { subscribe( iss ); }
        virtual ~SubscriberMixin() {}
        void subscribe( Issuer & iss ) {
            if( _issuer ) { this->unsubscribe(); }
            _issuer = &iss; _issuer->subscribe( *this );
        }
        void unsubscribe() {
            if( _issuer ) { _issuer->unsubscribe(*this); }
        }
        inline void receive() {
            _V_receive();
        }
        inline Issuer & issuer() {
            if( !_issuer ) {
                emraise(badState,
                  "Subscriber unsubscribed while issuer acquizition invoked.");
            }
            return *_issuer;
        }
        inline const Issuer & issuer() const {
            if( !_issuer ) {
                emraise(badState,
                  "Subscriber unsubscribed while issuer acquizition invoked.");
            }
            return *_issuer;
        }
    };
private:
    std::list<SubscriberMixin*> _subscribers;
public:
    void subscribe( SubscriberMixin & subs ) {
        if( _subscribers.end() !=
            std::find( _subscribers.begin(), _subscribers.end(), &subs ) ) {
                emraise(nonUniq,
                  "Repitative subscription of the same object %p.",
                  (void *) &subs);
        }
        _subscribers.push_back(&subs);
    }
    void unsubscribe( SubscriberMixin & subs ) {
        DECLTYPE(_subscribers)::iterator it =
            std::find( _subscribers.begin(), _subscribers.end(), &subs );
        if( _subscribers.end() == it ) {
            emraise(noSuchKey,
                  "Has no subscriber %p to unsubscribe.",
                  (void *) &subs);
        }
        _subscribers.push_back(&subs);
    }

    size_t n_subscribers() const { return _subscribers.size(); }

    void dispatch_all() {
        for( auto it  = _subscribers.begin();
                  it != _subscribers.end(); ++it ) {
            (*it)->receive();
        }
    }
    virtual ~IssuerMixin(){}
};

}  // namespace mixins
}  // namespace goo

# endif

