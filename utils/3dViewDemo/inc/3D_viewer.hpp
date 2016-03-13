# ifndef H_FL3D_VIEWER_H
# define H_FL3D_VIEWER_H

# include "goo_config.h"

# include "goo_ui/ogl_camera.hpp"

# if 0
# ifdef FLTK_FOUND

// development note: I have heard that double buffered windows
//    with OpenGL inside may cause obscure bugs but still not
//    faced with it
//# define NDOUBLE_WINDOW
//
// TODO: the viewer instance without color ruler and frame ruler
// can have SEGFAULT issues due to unforbidden functional routines
// thar remained available after last development stage.


# include <FL/Fl.H>
# include <FL/Fl_Gl_Window.H>
# ifndef NDOUBLE_WINDOW
# include <FL/Fl_Double_Window.H>
# endif
# include <FL/Enumerations.H>
# include <FL/Fl_Value_Slider.H>
# include <FL/Fl_Menu_Bar.H>
# include <FL/Fl_Check_Button.H>
# include <FL/Fl_Button.H>
# include <FL/Fl_Tree.H>
# include <FL/Fl_Spinner.H>
# include <FL/Fl_Light_Button.H>
# include <FL/Fl_Menu_Button.H>
# include <FL/gl.h>
# include <GL/glu.h>

# include <string.h>
# include <unordered_set>
# include "3D/hph_camera.tcc"
# include "hph_ogl_mesh.hpp"
# include "fl_frame_slider.hpp"
# include "fl_color_ruler.hpp"

namespace hph {
namespace ui {

//
// 3D OGLViewport
//

class OGLViewer;

class OGLViewport : public Fl_Gl_Window {
public:
    static struct Settings {
        hph_RGBA background;
        hph_RGBA labels;
        Linestyle wireframe;
        Linestyle meshBorder;
    } settings;
protected:
    Fl_Menu_Button * mnuViewport;
    OGLViewer * viewerInstancePtr;
protected:
    OGLCamera cam;
    void _init_viewport();
    int  _handle_key_event( int keyCode );
    UShort oldDragX, oldDragY;
    void _draw_meshes();
    void _draw_spatial_labels();
    void _draw_hover_labels();
public:
    OGLViewport( UShort X, UShort Y, UShort W, UShort H );
    ~OGLViewport();
    void draw();
    void resize(int X,int Y,int W,int H);
    int  handle( int event );
    virtual void set_viewer_instance( OGLViewer * );
};

//
// 3D viewer window
//

class OGLViewer : public 
    # ifndef NDOUBLE_WINDOW
    Fl_Double_Window
    # else
    Fl_Window
    # endif
{
public:
    enum {
        ENABLE_COLOR_RULER      = 0x1,
        ENABLE_DROPDOWN_MENU    = 0x2,
        ENABLE_TIME_RULER       = 0x4,
    } flags;
private:
    OGLViewport * glWin;
    FrameSlider * sldrNFrame;
    ColorRuler  * clrRuler;
protected:
    virtual void _V_change_frame( Size nframe ) { glWin->redraw(); }
    /// -1 for single snapshot, 0..N -- for video frames.
    virtual std::string _V_default_snapshot_filename( long nFrame ) const;
    /// Generates video according to functions above and deletes frames files.
    virtual void _V_make_video() const;

    /// The filename parameter can be given null to be obtained from elsewhere.
    static void take_picture(Fl_Widget * userdata, const char * filename);
    /// The filename parameter can be given null to be obtained from elsewhere.
    static void take_video(Fl_Widget * userdata, const char * filename);
    static void change_frame( Fl_Widget *, void * userdata );
public:
    std::unordered_set<g3D::OGLMesh *> meshesList;
    std::vector<std::string> userInfo;
public:
    OGLViewer( UShort W, UShort H,
               UShort rightMargin=0,
               UShort downMargin=0,
               UByte flags = 0,
               const char * L=nullptr );
    virtual inline void set_frames_list( const FrameSlider::FramesList & fl ) {
        sldrNFrame->set_frames_list(fl);}

    virtual void update_user_info() {}
    virtual void update_meshes() {}
    virtual void draw_custom() {}
    virtual void update_ruler_bounds( Float8 min, Float8 max ) { 
        assert(clrRuler);
        clrRuler->set_bounds(min, max);
        clrRuler->redraw(); }

    virtual inline void redraw_viewport(){
        assert(glWin); glWin->redraw(); }
    virtual inline hph_Palette & ruler_palette(){ 
        assert(clrRuler); return clrRuler->get_palette(); }
};

}  // namespace ui
}  // namespace hph

# else   // TODO: GLFW
# error "Sorry, but GLFW is unsupported for a while. You can use a FLTK viewer, or completly disable Goo's UI support."
# endif  // FLTK_FOUND
# endif
# endif  // H_FL3D_VIEWER_H

