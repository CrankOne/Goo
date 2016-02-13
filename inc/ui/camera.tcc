# ifndef H_GOO_CAMERA_H
# define H_GOO_CAMERA_H

# include "goo_quaternion.tcc"

// implementation based on:
// http://content.gpwiki.org/index.php/OpenGL:Tutorials:Using_Quaternions_to_represent_rotation

namespace hph {
namespace g3D {

template<typename Vector,
         typename ValT>
class iCamera {
public:
    struct Config {
        ValT        _nearCut, _farCut, _aperture;
        Vector      _LAt;   ///< Look-At point (global)
        Vector      _eye;   ///< Camera position point (global)
        Vector      _upV;

        ValT _apertureStep, _rollStep,
             _pitchStep,    _yawStep,
             _moveStep;
    };
    typedef Quaternion<ValT, Vector> CamQuaternion;
protected:
    Config      _original;
    Config      _current;
    ValT        _rMat[3][3];  ///< Rotation matrix cache.
    void _recache_rotation_matrix();

    virtual void _V_update(Float8 width, Float8 height) const = 0;
public:
    iCamera( const Config & cfg ) : _original(cfg), _current(cfg) {}
    virtual ~iCamera() {}

    /// Updates camera accordingly to current engine
    virtual void update(Float8 width, Float8 height) const { _V_update(width, height); }

    virtual void reset() { _current = _original; }

    /// Changes aperture angle.
    void aperture( const ValT & n ) {
        if( _current._aperture < 175. )
            _current._aperture += _current._apertureStep*n;
    }

    /// Moves camera on given offset vector.
    void global_move_eye_on( const Vector & delta ) {
        _current._eye += delta;
    }
    /// Moves Look-At point on given vector.
    void global_move_LAt_on( const Vector & delta ) {
        _current._LAt += delta;
    }

    /// Moves camera on given offset vector in local space.
    void local_move_eye_on( const Vector & delta ) {
        _current._eye += delta;
    }

    /// Moves look-at-point on given vector in local space.
    void local_move_LAt_on( const Vector & delta ) {
        _current._LAt += delta;
    }

    // Camera move along its view direction (local Y).
    void move_along_sight( const ValT sc ) {
        Vector newSight = sight_vector()*_current._moveStep*(1-sc);
        _current._eye += newSight;
    }

    // Move perpendicular sight line (local X).
    void move_in_sight_plane( Float8 dx, Float8 dy ) {
        Vector s = sight_vector();
        Vector e2 = _current._upV.ort(),
               e1 = (s.dot(e2)).ort(),
               d = ( e1*dx + e2*dy );

        _current._LAt += d*2*_current._moveStep*sin(_current._aperture/2);
        _current._eye += d*2*_current._moveStep*sin(_current._aperture/2);
    }

    inline Vector sight_vector() const {
        return _current._LAt - _current._eye;
    }

    /// Rotates along local X.
    void roll( const ValT angle ) {
        CamQuaternion cQ = rotation_quaternion( angle, sight_vector() );
        _current._upV = ((cQ*_current._upV)*cQ.invert()).normalize().vec();
    }

    /// Rotates along local Y.
    void pitch( const ValT angle, bool aroundLAt=true ) {
        auto axis = (sight_vector().dot(_current._upV)).ort();
        CamQuaternion cQ = rotation_quaternion( angle, axis );
        Vector newSight = ((cQ*sight_vector())*cQ.invert()).vec(),
               newUp = ((cQ*_current._upV)*cQ.invert()).vec();
        if( aroundLAt ) {
            _current._eye = _current._LAt - newSight;
            _current._upV = newUp;
        } else {
            _TODO_  // TODO
        }
    }

    /// Rotates along local Z.
    void yaw( const ValT angle, bool aroundLAt=true ) {
        CamQuaternion cQ = rotation_quaternion( angle, _current._upV );
        Vector newSight = ((cQ*sight_vector())*cQ.invert()).vec();
        if( aroundLAt ) {
            _current._eye = _current._LAt - newSight;
        } else {
            _TODO_  // TODO
        }
    }

    // Produces rotation quaternion
    inline CamQuaternion rotation_quaternion( ValT angle, const Vector & axis_ ) const {
        Vector axis = axis_.ort();
        return CamQuaternion( cos(angle/2), axis*sin(angle/2) );
    }

    // Increases roll angle -- rotate around local X.
    inline void roll_inc( const ValT sc=1. ) {
        roll(   _current._rollStep*sc );
    }
    // Decreases roll angle -- rotate around local X.
    inline void roll_dec( const ValT sc=1. ) {
        roll( - _current._rollStep*sc );
    }
    // Increases yaw angle -- rotate around local Z. 
    inline void yaw_inc( const ValT sc=1. ) {
        yaw( _current._yawStep*sc );
        // TODO: move LAt-point
    }
    // Decreases yaw angle -- rotate around local Z. 
    inline void yaw_dec( const ValT sc=1. ) {
        yaw( -_current._yawStep*sc );
        // TODO: move LAt-point
    }
    // Increases pitch angle -- rotate around local Y.
    inline void pitch_inc( const ValT sc=1. ) {
        pitch(  _current._pitchStep*sc );
        // TODO: move LAt-point
    }
    // Decreases pitch angle -- rotate around local Y.
    inline void pitch_dec( const ValT sc=1. ) {
        pitch( -_current._pitchStep*sc );
        // TODO: move LAt-point
    }
};


template<typename Vector,
         typename ValT> void
iCamera<Vector, ValT>::_recache_rotation_matrix() {
    const CamQuaternion q;
    const ValT x = q.vec().x(),
               y = q.vec().y(),
               z = q.vec().z(),
               w = q.m();
    _rMat = {
        { 1 - 2*y*y - 2*z*z,        2*x*y - 2*z*w,          2*x*z + 2*y*w },
        {     2*x*y + 2*z*w,    1 - 2*x*x - 2*z*z,          2*y*z - 2*x*w },
        {     2*x*z - 2*y*w,        2*y*z + 2*x*w,      1 - 2*x*x - 2*y*y }
    };
}

} // namespace g3D
} // namespace hph

# endif  // H_GOO_CAMERA_H

