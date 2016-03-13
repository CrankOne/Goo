# include "goo_ui/ogl_camera.hpp"

# ifdef OPENGL_GLU_FOUND

# include <GL/gl.h>
# include <GL/glu.h>

namespace goo {
namespace g3D {

OGLCamera::Parent::Config OGLCamera::defaultCamCfg;

void
OGLCamera::_V_update(Float8 width, Float8 height) const {
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    gluPerspective( _current._aperture,
        (GLfloat)width/(GLfloat)height,
        _current._nearCut, _current._farCut );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    g3D::Vector3Dim<g3D::Real> eye = _current._LAt + _current._eye;
    gluLookAt( eye.x(),             eye.y(),            eye.z(), // TODO
               _current._LAt.x(),   _current._LAt.y(),  _current._LAt.z(),
               _current._upV.x(),   _current._upV.y(),  _current._upV.z() );
}

}  // namespace g3D
}  // namespace goo

# endif  // OPENGL_FOUND

