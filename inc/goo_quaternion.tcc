# ifndef H_GOO_QUATERNION_H
# define H_GOO_QUATERNION_H

# include "goo_mixins.tcc"

namespace goo {
namespace g3D {

template<typename RealT,
         typename Vector3T>
class Quaternion : public goo::mixins::PlusOp<
                                    Quaternion<RealT, Vector3T>,
                                    Quaternion<RealT, Vector3T> >,
                   public goo::mixins::DashOp<
                                    Quaternion<RealT, Vector3T>,
                                    Quaternion<RealT, Vector3T> >,
                   public goo::mixins::SlashOp<
                                    Quaternion<RealT, Vector3T>,
                                    RealT >,
                   public goo::mixins::AsteriskOp<
                                    Quaternion<RealT, Vector3T>,
                                    RealT >,
                   public goo::mixins::AsteriskOp<
                                    Quaternion<RealT, Vector3T>,
                                    Vector3T >,
                   public goo::mixins::AsteriskOp<
                                    Quaternion<RealT, Vector3T>,
                                    Quaternion<RealT, Vector3T> > {
public:
    typedef RealT Real;
    typedef Vector3T Vector3;
    using goo::mixins::AsteriskOp<Quaternion, Real >::operator*;
    using goo::mixins::AsteriskOp<Quaternion, Quaternion >::operator*;
    using goo::mixins::AsteriskOp<Quaternion, Vector3 >::operator*;
protected:
    Real w;
    Vector3 v;
public:
    explicit Quaternion(){}
    explicit Quaternion( Real w_, const Vector3 & v_ ) : w(w_), v(v_){}
    virtual inline void operator+= ( const Quaternion & t ) {
        w += t.w;
        v += t.v;
    }
    virtual inline void operator-= ( const Quaternion & t ) {
        w -= t.w;
        v -= t.v;
    }
    virtual inline void operator/= ( const Real & t ) {
        w /= t;
        v /= t;
    }
    virtual inline void operator*= ( const Real & t ) {
        w *= t;
        v *= t;
    }
    virtual inline void operator*= ( const Quaternion & t ) {
        w = w*t.w - v.prod(t.v);
        v = v*t.w + t.v*w + v.dot(t.v);
    }
    virtual inline void operator*= ( const Vector3 & t ) {
        Real newW = v.prod(t);
        v = t*w + v.dot(t);
        w = newW;
    }
    /// Computes norm.
    Real norm() const;
    /// Normalizes this quaternion.
    Quaternion & normalize();

    /// Writes conjugated (to this) quaternion.
    void conj( Quaternion & ) const;
    /// Returns conjugated quaternion.
    inline Quaternion conj() const { Quaternion q; conj(q); return q; }

    /// Computes and writes $q^{-1}$.
    void invert( Quaternion & ) const;
    /// Returns $q^{-1}$.
    Quaternion invert() const { Quaternion q; invert(q); return q; }

    inline Real m() const { return w; }
    inline void m( Real m ) { w = m; }
    inline const Vector3 & vec() const { return v; }
    inline Vector3 & vec() { return v; }
};  // class Quaternion

template<typename RealT, typename Vector3T> Quaternion<RealT, Vector3T> &
Quaternion<RealT, Vector3T>::normalize() {
	Real d = norm();
	w /= d;
	v /= d;
    return *this;
}

template<typename RealT, typename Vector3T> RealT
Quaternion<RealT, Vector3T>::norm() const {
    return sqrt(v.x()*v.x() + v.y()*v.y() +
                v.z()*v.z() + w*w);
}

template<typename RealT, typename Vector3T> void
Quaternion<RealT, Vector3T>::conj( Quaternion & q ) const {
    q.w = w;
    q.v = v*(-1);
}

template<typename RealT, typename Vector3T> void
Quaternion<RealT, Vector3T>::invert( Quaternion & q ) const {
    q = this->conj()/pow(this->norm(), 2);
}

}  // namespace g3D
}  // namespace goo

# endif  // H_GOO_QUATERNION_H

