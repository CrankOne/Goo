# ifndef H_GOO_OGL_CAMERA_H
# define H_GOO_OGL_CAMERA_H

# include "goo_ui/camera.tcc"
# include "goo_ui/spatial_vector.tcc"

# ifdef OPENGL_GLU_FOUND

namespace goo {
namespace g3D {

class OGLCamera : public goo::g3D::iCamera<g3D::Vector3Dim<g3D::Real>, g3D::Real> {
public:
    typedef goo::g3D::iCamera<g3D::Vector3Dim<g3D::Real>, g3D::Real> Parent;

    static Parent::Config defaultCamCfg;
protected:
    virtual void _V_update(Float8 width, Float8 height) const override;
public:
    OGLCamera() : Parent(defaultCamCfg) {}
};

}  // namespace g3D
}  // namespace goo

# endif  // OPENGL_GLU_FOUND

# endif  // H_GOO_OGL_CAMERA_H

