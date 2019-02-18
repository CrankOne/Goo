# ifndef GOO_SPATIAL_VECTOR_HPP
# define GOO_SPATIAL_VECTOR_HPP

// Note: avoid C++11 ambigious operator shadowing:
// http://stackoverflow.com/questions/4831759/ambiguous-definition-of-operator-with-multiple-inheritance
// TODO: unary arithmetic operators like *=(...) should retern this-reference.

# include "math.h"
# include "goo_mixins/basicOperators.tcc"

namespace goo {
namespace g3D {

template<typename CarrierT>
class Vector3Dim : public goo::mixins::PlusOp<
                                    Vector3Dim<CarrierT>,
                                    Vector3Dim<CarrierT> >,
                   public goo::mixins::DashOp<
                                    Vector3Dim<CarrierT>,
                                    Vector3Dim<CarrierT> >,
                   public goo::mixins::SlashOp<
                                    Vector3Dim<CarrierT>,
                                    CarrierT >,
                   public goo::mixins::AsteriskOp<
                                    Vector3Dim<CarrierT>,
                                    CarrierT >{
protected:
    CarrierT c[3];
public:
    typedef Vector3Dim<CarrierT> SelfType;
    typedef CarrierT CarrierType;


    virtual inline void operator+= ( const SelfType & t ) {
        c[0] += t.c[0]; c[1] += t.c[1]; c[2] += t.c[2];
    }
    virtual inline void operator-= ( const SelfType & t ) {
        c[0] -= t.c[0]; c[1] -= t.c[1]; c[2] -= t.c[2];
    }
    virtual inline void operator/= ( const CarrierT & t ) {
        c[0] /= t; c[1] /= t; c[2] /= t;
    }
    virtual inline void operator*= ( const CarrierT & t ) {
        c[0] *= t; c[1] *= t; c[2] *= t;
    }

    inline const CarrierT & x() const { return c[0]; }
    inline const void x( const CarrierT & v ) { c[0] = v; }
    inline const CarrierT & y() const { return c[1]; }
    inline const void y( const CarrierT & v ) { c[1] = v; }
    inline const CarrierT & z() const { return c[2]; }
    inline const void z( const CarrierT & v ) { c[2] = v; }
    inline CarrierT & operator[](UByte i) { return c[i]; }
    inline const CarrierT & operator[](UByte i) const { return c[i]; }


    explicit Vector3Dim() {
        //c = {0,0,0};
        c[0] = c[1] = c[2] = 0.;
    }
    Vector3Dim(const Vector3Dim & o) {
        //c = {o.c[0], o.c[1], o.c[2]};
        c[0] = o.c[0];
        c[1] = o.c[1];
        c[2] = o.c[2];
    }
    explicit Vector3Dim( const CarrierT * i ) {
        //c = {i[0], i[1], i[2]};
        c[0] = i[0];
        c[1] = i[1];
        c[2] = i[2];
    }
    explicit Vector3Dim( const CarrierT & ix,
                const CarrierT & iy,
                const CarrierT & iz ) {
        c[0] = ix;
        c[1] = iy;
        c[2] = iz;
    }
    Vector3Dim& operator=( const Vector3Dim& rhs ) {
        c[0] = rhs.c[0];
        c[1] = rhs.c[1];
        c[2] = rhs.c[2];
        return *this;
    }

    inline void zero() {
        c[0] = c[1] = c[2] = 0; // todo: use XOR for simple carriers?
    }

    # define X c[0]
    # define Y c[1]
    # define Z c[2]

    virtual SelfType dot( const SelfType & o ) const {
        return SelfType(
                Y*o.Z - Z*o.Y,
                Z*o.X - X*o.Z,
                X*o.Y - Y*o.X
            );
    }

    SelfType ort() const {
        return (*this)/norm();
    }

    virtual CarrierT norm() const {
        return sqrt( X*X + Y*Y + Z*Z );
    }

    virtual inline CarrierType prod( const Vector3Dim<CarrierT> & t ) const {
        return c[0]*t[0] + c[1]*t[1] + c[2]*t[2];
    }

    CarrierT euler_phi() const {
        if(X) {
            return atan( Y/X );
        } else {
            return M_PI/2*(Y > 0 ? 1 : -1);
        }
    }

    void euler_phi( const CarrierT & phi ) {
        SelfType copy = (*this);
        X = copy.X*cos(phi) - copy.Y*sin(phi);
        Y = copy.Y*cos(phi) + copy.X*sin(phi);
    }

    CarrierT euler_theta() const {
        SelfType ort_ = ort();
        return atan( ort_.Z/sqrt( ort_.Y*ort_.Y + ort_.X*ort_.X ) );
    }

    void euler_theta( const CarrierT & theta ) {
        SelfType copy = (*this);
        Y = copy.Y*cos(theta) - copy.Z*sin(theta);
        Z = copy.Z*cos(theta) + copy.Y*sin(theta);
    }

    SelfType rotate( const CarrierT a_, const SelfType & axis ) {
        double a = a_*M_PI/180;
        const double ax = axis.x(),
                     ay = axis.y(),
                     az = axis.z();
        double M[3][3] = {
            { cos(a)+(1-cos(a))*ax*ax,       (1-cos(a))*ax*ay - az*sin(a),    (1-cos(a))*ax*az + ay*sin(a) },
            { (1-cos(a))*ay*ax+az*sin(a),    cos(a) + (1-cos(a))*ay*ay,        (1-cos(a))*ay*az - ax*sin(a) },
            { (1-cos(a))*az*ax - ay*sin(a),  (1-cos(a))*az*ay + ax*sin(a),    cos(a)+(1-cos(a))*az*az     }
        };
        SelfType upV = *this;
        upV.x( M[0][0]*upV.x() + M[1][0]*upV.y() + M[2][0]*upV.z() );
        upV.y( M[0][1]*upV.x() + M[1][1]*upV.y() + M[2][1]*upV.z() );
        upV.z( M[0][2]*upV.x() + M[1][2]*upV.y() + M[2][2]*upV.z() );
        return upV;
    }

    const CarrierT * storage() const {
        return c;
    }

    CarrierT * storage() {
        return c;
    }
    # undef X
    # undef Y
    # undef Z
};

} // namespace g3D
} // namespace goo

# endif  // GOO_SPATIAL_VECTOR_HPP

