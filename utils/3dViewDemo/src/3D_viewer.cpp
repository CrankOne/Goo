# include "3D_viewer.hpp"

# if 1
//# include "fl_frame_slider.hpp"
# include "fl_3D_viewer.hpp"
//# include "hph_ui_utils.hpp"

# define PNG_DEBUG 3
# include <png.h>

namespace hph {
namespace ui {

//
// OGLViewport
//

OGLViewport::Settings OGLViewport::settings = {
    /* background ...... */ {128,128,128,255},
    /* labels .......... */ {  0,  0,  0,255},
    /* wireframe ....... */ {{128,128,128,255}, 0.5},
    /* meshBorder ...... */ {{  0,  0,  0,255}, 1.5},
};

void
OGLViewport::_init_viewport() {
    glClearColor( settings.background.R/255.,
                  settings.background.G/255.,
                  settings.background.B/255.,
                  settings.background.A/255.);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH); // NOTE: for interpolating lines colors instead of GL_FLAT
    glDepthFunc(GL_LEQUAL);
    //glutInitDisplayMode(GLUT_SINGLE|GLUT_RGBA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    valid(1);
}

int
OGLViewport::_handle_key_event( int keyCode ) {
    switch( keyCode ) {
        case 'd' : {cam.yaw_inc();  } break;
        case 'a' : {cam.yaw_dec();  } break;
        case 's' : {cam.pitch_inc();} break;
        case 'w' : {cam.pitch_dec();} break;
        case 'q' : {cam.roll_inc(); } break;
        case 'e' : {cam.roll_dec(); } break;
        case 'r' : {cam.reset();}     break;
        // TODO...
        default:
            return 0;
    };
    return 1;
}


void
OGLViewport::draw() {  // TODO: fancy error reporting.
    if (!valid()) {
        _init_viewport();
    }
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    GLenum err = glGetError();
    if ( err != GL_NO_ERROR ) {
        eprintf("GLGETERROR=%d after clearing.\n", (int)err);
    }
    cam.update( w(), h() );
    err = glGetError();
    if ( err != GL_NO_ERROR ) {
        eprintf("GLGETERROR=%d after cam updating\n", (int)err);
    }
    _draw_meshes();
    err = glGetError();
    if ( err != GL_NO_ERROR ) {
        eprintf("GLGETERROR=%d afterdrawing meshes.\n", (int)err);
    }
    if( viewerInstancePtr ) {
        viewerInstancePtr->draw_custom();
            GLenum err = glGetError();
        if ( err != GL_NO_ERROR ) {
            eprintf("GLGETERROR=%d after drawing custom.\n", (int)err);
        }
    }
    glColor3f(0., 0., 0. );
    glDisable(GL_TEXTURE_2D);

    glDisable(GL_DEPTH_TEST);
    _draw_spatial_labels();
    glLoadIdentity();
    _draw_hover_labels();
    glEnable(GL_DEPTH_TEST); //?

    err = glGetError();
    if ( err != GL_NO_ERROR ) {
        eprintf("GLGETERROR=%d after drawing all.\n", (int)err);
    }
}


int
OGLViewport::handle( int event ) {
    switch(event) {
        case FL_PUSH: {
            //... mouse down event ...
            //... position in Fl::event_x() and Fl::event_y()
            this->take_focus();
            if( Fl::event_button() == FL_RIGHT_MOUSE ) {
                mnuViewport->show();
                return 1;
            }
            return 1;
        } break;
        case FL_MOUSEWHEEL: {
            if( Fl::event_command() ) {
                cam.aperture( - Fl::event_dy() );
            } else if( Fl::event_dy() > 0 ) {
                cam.move_along_sight(  .95 );
            } else {
                cam.move_along_sight( 1.05 );
            }
            redraw();
            return 1;
        } break;
        case FL_DRAG: {
            //... mouse moved while down event ...
            // Fl::event_state() -- button
            if( FL_BUTTON2 & Fl::event_state() ) {
                if( !oldDragX ) {
                    oldDragX = Fl::event_x_root();
                    oldDragY = Fl::event_y_root();
                    this->cursor(FL_CURSOR_NONE);
                    return 1;
                }
                SShort dxI = oldDragX - Fl::event_x_root(),
                       dyI = oldDragY - Fl::event_y_root();
                double dx = double(dxI)/w(),
                       dy = double(dyI)/h();
                if( FL_SHIFT & Fl::event_state() ) {
                    cam.yaw_inc( dx*50 );
                    cam.pitch_inc( dy*50 );
                } else {
                    cam.move_in_sight_plane( dx, -dy );
                }
                set_cursor_position_to(
                    oldDragX,
                    oldDragY );
                redraw();
            }
            return 1;
        } break;
        case FL_RELEASE: {
            if( FL_MIDDLE_MOUSE == Fl::event_button() ) {
                oldDragX = 0;
                oldDragY = 0;
                this->cursor(FL_CURSOR_DEFAULT);
                //warning( "RELEASE EVENT #1\n" ); //XXX
            }
            //warning( "RELEASE EVENT\n" ); //XXX
            return 1;
        } break;
        case FL_FOCUS :
        case FL_UNFOCUS : {
            //this->invalidate();
            //... Return 1 if you want keyboard events, 0 otherwise
            return 1;
        } break;
        case FL_KEYBOARD: {
            if(_handle_key_event( Fl::event_key() ) ) {
                cam.update( w(), h() );
                redraw();
                return 1;
            }
        } break;
        case FL_SHORTCUT: {
            //... shortcut, key is in Fl::event_key(), ascii in Fl::event_text()
            //... Return 1 if you understand/use the shortcut event, 0 otherwise...
            return 1;
        } break;
    }
    return Fl_Gl_Window::handle(event);
}


void
OGLViewport::_draw_meshes( ) {
    glBegin(GL_LINES);
        glColor3d(1,0,0);
        glVertex3f(  0.0f, 0.0f, 0.0f);
        glVertex3f(  1.0f, 0.0f, 0.0f);
    glEnd();

    glBegin(GL_LINES);
        glColor3d(0,1,0);
        glVertex3f(  0.0f, 0.0f, 0.0f);
        glVertex3f(  0.0f, 1.0f, 0.0f);
    glEnd();

    glBegin(GL_LINES);
        glColor3d(0,0,1);
        glVertex3f(  0.0f, 0.0f, 0.0f);
        glVertex3f(  0.0f, 0.0f, 1.0f);
    glEnd();

    if( viewerInstancePtr ) {
        viewerInstancePtr->update_meshes();
        for( auto it  = viewerInstancePtr->meshesList.begin();
                  it != viewerInstancePtr->meshesList.end(); ++it ) {
            (*it)->draw();
        }
    }
}

void
OGLViewport::_draw_spatial_labels() {
    {
        const char *p;
        gl_font(1, 12);
        glColor3f(0.0, 0.0, 0.0);
        glRasterPos3f(0, 0, 0); 
        p = ".O";
        gl_draw(p, strlen(p));
    }
}

void
OGLViewport::_draw_hover_labels() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, this->w(), 0 , this->h(), -1, 1 );
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if(viewerInstancePtr) {
        viewerInstancePtr->update_user_info();
        UByte nLine = 1;
        for( std::vector<std::string>::const_iterator it = viewerInstancePtr->userInfo.begin();
                it != viewerInstancePtr->userInfo.end(); ++it, ++nLine) {
            glRasterPos2d( 10, this->h() - 10 - 14*nLine );
            gl_draw( it->c_str() );
        }
    }
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
}

OGLViewport::OGLViewport( UShort X, UShort Y, UShort W, UShort H ) :
        Fl_Gl_Window(X, Y, W, H, null ),
        viewerInstancePtr( nullptr ),
        oldDragX(0), oldDragY(0) {
    cam.reset();
    // TODO:
    mnuViewport = new Fl_Menu_Button(0, 0, this->w(), this->h());
    mnuViewport->type(Fl_Menu_Button::POPUP3);
    mnuViewport->add("This|is|a popup|menu");
}

OGLViewport::~OGLViewport() {
}

void
OGLViewport::resize(int X,int Y,int W,int H) {
    Fl_Gl_Window::resize(X,Y,W,H);
    glLoadIdentity();
    glViewport(0,0,W,H);
    glOrtho(-W,W,-H,H,-1,1);
    cam.update(w(), h());
    redraw();
}

void
OGLViewport::set_viewer_instance( OGLViewer * instPtr ) {
    viewerInstancePtr = instPtr;
}

//
// OGLViewer
//

OGLViewer::OGLViewer( UShort W, UShort H,
                      UShort rightMargin,
                      UShort downMargin,
                      UByte flags, const char * L ) :
            # ifndef NDOUBLE_WINDOW
            Fl_Double_Window
            # else
            Fl_Window
            # endif
            ( W, H,
              (L ? L : "Hephaestus 3D Viewer Application")
            ) {
    Fl::gl_visual(FL_RGB);
    const UShort padding = 5,
                 colorRulerWidth = 40,
                 timeRulerHeight = 30;

    UShort viewportX = ( (flags & ENABLE_COLOR_RULER) ? 
                            colorRulerWidth + padding : 0 ),
           viewportW = W - viewportX - rightMargin /*- padding*/,
           viewportH = H - ( (flags & ENABLE_TIME_RULER)  ? 
                                timeRulerHeight + padding : 0 ) - downMargin /*- 2*padding*/;

    if( flags & ENABLE_COLOR_RULER ) {
        clrRuler = new ColorRuler( 0, 0, colorRulerWidth, viewportH );
    } else {
        clrRuler = nullptr;
    }

    if( flags & ENABLE_TIME_RULER ) {
        sldrNFrame = new FrameSlider(
                  viewportX,
                  H - timeRulerHeight - downMargin,
                  viewportW, timeRulerHeight );
        sldrNFrame->callback( change_frame, this );
        sldrNFrame->set_picturing( this, take_picture, take_video );
    } else {
        sldrNFrame = nullptr;
    }

    // OpenGL window
    glWin = new OGLViewport( viewportX, 0/*padding*/,
                             viewportW, viewportH );
    glWin->set_viewer_instance(this);
    this->resizable( glWin );
}

//
// Viewer UI Callbacks
//

void
OGLViewer::take_picture(Fl_Widget * this_, const char * filename) {
    # if 1
    OGLViewer * appwin = dynamic_cast<OGLViewer *>(this_);
    appwin->glWin->redraw();
    GLint vp[4];
    glGetIntegerv( GL_VIEWPORT, vp );
    # if 0
    dprintf( "Viewport dimensions  : {%d,%d} %dГ—%d.\n", vp[0], vp[1], vp[2], vp[3] );
    dprintf( "OGL Widget dimensions: {%d,%d} %dГ—%d.\n", appwin->glWin->x(),
                                                        appwin->glWin->y(),
                                                        appwin->glWin->w(),
                                                        appwin->glWin->h());
    # endif
    UShort oglCtxW    = vp[2],
           oglCtxH    = vp[3],
           imgWidth   = appwin->clrRuler->w() + oglCtxW,
           imgHeight  = appwin->clrRuler->h();
    assert( appwin->clrRuler->h() == oglCtxH );
    if( !appwin ) {
        hraise(badArchitect, "bad type provided");
    }
    
    hph_RGB * pic = new hph_RGB[ imgHeight*imgWidth ];
    png_bytep * rowPtrs = new png_bytep[imgHeight];
    for( UShort i = 0; i < imgHeight; ++i ) {
        rowPtrs[i] = (png_bytep) (
                pic + (imgHeight-i-1)*imgWidth);
    }
    {
        appwin->glWin->make_current();
        hph_RGB * glPic = new hph_RGB[ (oglCtxW)*(oglCtxH) ];

        glReadBuffer(GL_BACK);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ROW_LENGTH, oglCtxW);

        glReadPixels(
                vp[0], vp[1],
                oglCtxW, oglCtxH,
                GL_RGB, GL_UNSIGNED_BYTE,
                glPic
            );
        const UByte * cR = appwin->clrRuler->get_img();
        assert(cR);
        for( UShort i = 0; i < imgHeight; ++i ) {
            memcpy( pic + i*imgWidth, cR + (imgHeight-i-1)*appwin->clrRuler->w()*3,
                    sizeof(hph_RGB)*(appwin->clrRuler->w()) );
            memcpy( pic + i*imgWidth + appwin->clrRuler->w(),
                    glPic + i*(oglCtxW),
                    sizeof(hph_RGB)*(oglCtxW) );
        }
        delete [] cR;
        delete [] glPic;
    }

    std::string fname;
    if(!filename) {
        fname = appwin->_V_default_snapshot_filename( -1 );
    } else {
        fname = std::string( filename );
    }

    FILE *fp = fopen(fname.c_str(), "wb");
    png_structp png_ptr = png_create_write_struct(
                              PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    if(setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during init_io.\n");
    } png_init_io(png_ptr, fp);

    if (setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during writing header.\n");
    } png_set_IHDR(png_ptr, info_ptr, imgWidth, imgHeight,
                 /* bit_depth  */ 8,
                 /* color_type */ PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
      //png_set_text( png_ptr, info_ptr, text, num_text );
      png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during writing bytes.\n");
    } png_write_image(png_ptr, rowPtrs);

    if(setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during end of write.\n");
    } png_write_end(png_ptr, NULL);
    fclose(fp);
    png_destroy_info_struct( png_ptr, &info_ptr );
    delete [] pic;
    # endif
}

void
OGLViewer::take_video( Fl_Widget * this_, const char * filename) {
    # if 1
    OGLViewer * appwin = dynamic_cast<OGLViewer *>(this_);
    if( !appwin ) {
        hraise(badArchitect, "bad type provided");
    }

    FrameSlider::first_frame( nullptr, appwin->sldrNFrame );
    Size nFrame = 0;
    for( auto it = appwin->sldrNFrame->get_frames_list().begin();
              it != appwin->sldrNFrame->get_frames_list().end(); ++it, ++nFrame){
        FrameSlider::next_frame( nullptr, appwin->sldrNFrame );
        if( appwin->clrRuler ) {
            appwin->clrRuler->redraw();
        }
        Fl::flush();
        take_picture(appwin, appwin->_V_default_snapshot_filename(nFrame).c_str());
    }
    appwin->_V_make_video();
    # endif
}

void
OGLViewer::change_frame(Fl_Widget *w, void * userdata) {
    OGLViewer & oglv = *((OGLViewer*)userdata);
    if( oglv.sldrNFrame->align_value() ) {
        oglv._V_change_frame( oglv.sldrNFrame->value() );
    }
}

std::string
OGLViewer::_V_default_snapshot_filename( long nFrame ) const {
    if( nFrame < 0 ) {
        return "./snapshot.png";
    } else {
        char bf[48];
        snprintf( bf, 48, "./frame-%08zu.png", nFrame );
        return bf;
    }
}

void
OGLViewer::_V_make_video() const {
    char bf2[256];
    snprintf( bf2, 256, "ffmpeg -r 10 -f image2 -i ./frame-%%08d.png \
-vcodec mpeg4 -b:v 800k ./video.avi" );
    std::cout << bf2 << std::endl;
    int res = system( bf2 );
    if(res) { warning( "system command failure.\n" ); }
    res = system( "rm -rf ./frame-*.png" );
    if(res) { warning( "system command failure.\n" ); }
}


}  // namespace ui
}  // namespace hph

# if 0
void
GLCamera::reset() {
          _nearCut = io["Cam.near-cut"          ].as<Float8>();
           _farCut = io["Cam.far-cut"           ].as<Float8>();
         _aperture = io["Cam.aperture"          ].as<Float8>();
     _apertureStep = io["Cam.aperture-step"     ].as<Float8>();
            _LAT.x(  io["Cam.look-at-point-x"   ].as<Float8>() );
            _LAT.y(  io["Cam.look-at-point-y"   ].as<Float8>() );
            _LAT.z(  io["Cam.look-at-point-z"   ].as<Float8>() );
            _eye.x(  io["Cam.position-x"        ].as<Float8>() );
            _eye.y(  io["Cam.position-y"        ].as<Float8>() );
            _eye.z(  io["Cam.position-z"        ].as<Float8>() );
         _rollStep = io["Cam.roll-step"         ].as<Float8>();
        _pitchStep = io["Cam.pitch-step"        ].as<Float8>();
          _yawStep = io["Cam.yaw-step"          ].as<Float8>();
         _moveStep = io["Cam.move-step"         ].as<Float8>();
         _up.x(0); _up.y(0); _up.z(1);
}

void
GLCamera::update(Float8 width, Float8 height) const { // todo
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    gluPerspective( _aperture,
        (GLfloat)width/(GLfloat)height,
        _nearCut, _farCut );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    Vector eye = _LAT + _eye;
    gluLookAt( eye.x(),  eye.y(),  eye.z(), // todo: ?
              _LAT.x(), _LAT.y(), _LAT.z(),
               _up.x(),  _up.y(),  _up.z() );
}

//
// OpenGL window
//

void
OGLViewport::_init_viewport() {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH); // NOTE: for interpolating lines colors instead of GL_FLAT
    glDepthFunc(GL_LEQUAL);

    //glutInitDisplayMode(GLUT_SINGLE|GLUT_RGBA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    tracer->init_textures();

    valid(1);
}

int
OGLViewport::_handle_key_event( int keyCode ) {
    switch( keyCode ) {
        case 'a' : {cam.yaw_inc();  } break;
        case 'd' : {cam.yaw_dec();  } break;
        case 's' : {cam.pitch_dec();} break;
        case 'w' : {cam.pitch_inc();} break;
        case 'r' : {cam.reset();}     break;
        default:
            return 0;
    };
    return 1;
}


void
OGLViewport::draw() {
    if (!valid()) {
        _init_viewport();
    }
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    cam.update( w(), h() );
    // Draw objects

    ViewerAppWindow * appwin = dynamic_cast<ViewerAppWindow *>(this->window());
    if( !appwin ) {
        hraise(badArchitect, "bad type provided");
    }

    appwin->cDrawer( P, *tracer );
    appwin->imgRuler->redraw();

    glColor3f(0., 0., 0. );
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    {
        const char *p;
        gl_font(1, 12);
        glColor3f(0.0, 0.0, 0.0);
        glRasterPos3f(0, 0, 0); 
        p = ".O";
        gl_draw(p, strlen(p));
    }

    glLoadIdentity();

    std::vector<std::string> tokens;
    std::string varCopy = P.user_info();
    boost::split(tokens, varCopy, boost::is_any_of("\n"));
    UByte nLine = 1;
    for( DECLTYPE(tokens)::const_iterator it = tokens.begin();
            it != tokens.end(); ++it, ++nLine) {
        glRasterPos3f(-1.3, .8 - .8*nLine/10., -1.);
        gl_draw( it->c_str() );
    }
    glEnable(GL_DEPTH_TEST);
    // TODO: is_vector()/is_scalar()
    // TODO: other vertex tracers?
    
    if( tracer->is_vector_value() ) { 
        glDisable(GL_TEXTURE_2D);
    } else {
        glEnable(GL_TEXTURE_2D); // should be turned on for scalar (hull) drawing
        glEnable(GL_LINE_SMOOTH);
    }

    GLenum err = glGetError();
    if ( err != GL_NO_ERROR ) {
        eprintf("GLGETERROR=%d\n", (int)err);
    }

    //this->make_current();
    //this->swap_buffers();
    //this->redraw_overlay();
}


int
OGLViewport::handle( int event ) {
    switch(event) {
        case FL_PUSH: {
            //... mouse down event ...
            //... position in Fl::event_x() and Fl::event_y()
            this->take_focus();
            return 1;
        } break;
        case FL_MOUSEWHEEL: {
            if( Fl::event_command() ) {
                cam.aperture( - Fl::event_dy() );
            } else if( Fl::event_dy() > 0 ) {
                cam.move_along_sight(  .95 );
            } else {
                cam.move_along_sight( 1.05 );
            }
            redraw();
            return 1;
        } break;
        case FL_DRAG: {
            //... mouse moved while down event ...
            // Fl::event_state() -- button
            if( FL_BUTTON2 & Fl::event_state() ) {
                dprintf("#1\n"); //XXX
                if( !oldDragX ) {
                    oldDragX = Fl::event_x();
                    oldDragY = Fl::event_y();
                    return 1;
                }
                double dx = double(oldDragX - Fl::event_x())/w(),
                       dy = double(oldDragY - Fl::event_y())/h();
                set_cursor_position_to(
                        _owner.x() + oldDragX,
                        _owner.y() + oldDragY );
                oldDragX = Fl::event_x();
                oldDragY = Fl::event_y();
                //if( FL_SHIFT & Fl::event_state() ) {} // TODO: axial movement
                cam.move_in_sight_plane( -dx, -dy );
                redraw();
            }
            return 1;
        } break;
        case FL_RELEASE: {
            dprintf("#2\n"); //XXX
            if( FL_MIDDLE_MOUSE == Fl::event_button() ) {
                oldDragX = 0;
                oldDragY = 0;
                warning( "RELEASE EVENT #1\n" ); //XXX
            }
            warning( "RELEASE EVENT\n" ); //XXX
            return 1;
        } break;
        case FL_FOCUS :
        case FL_UNFOCUS : {
            //this->invalidate();
            //... Return 1 if you want keyboard events, 0 otherwise
            return 1;
        } break;
        case FL_KEYBOARD: {
            if(_handle_key_event( Fl::event_key() ) ) {
                cam.update( w(), h() );
                redraw();
                return 1;
            }
        } break;
        case FL_SHORTCUT: {
            //... shortcut, key is in Fl::event_key(), ascii in Fl::event_text()
            //... Return 1 if you understand/use the shortcut event, 0 otherwise...
            return 1;
        } break;
    }
    return Fl_Gl_Window::handle(event);
}

OGLViewport::OGLViewport( const ifw::InitObj & io, const ifw::iEqu & P_ ) :
        Fl_Gl_Window(55,5,
            io["Viewer.viewport-w"].as<UShort>(),
            io["Viewer.viewport-h"].as<UShort>(),
            null ),
        iObj(io),
        P(P_),
        cam(iObj),
        oldDragX(0), oldDragY(0) {
    cam.reset();
    tracer = P.create_tracer();
}

OGLViewport::~OGLViewport() {
}

//
// Application window
//

void
ViewerAppWindow::switch_variable( iVertexTracer::CuttingConfiguration * nConf ) {
    cCut = nConf;
    //appwin->get_glWin().get_tracer().invalidate_range( GLVertexTracer::?? ); // TODO?
    _refresh_cut_sliders();
}

void
ViewerAppWindow::change_palette(Fl_Widget * w, void * userdata) {
    ViewerAppWindow * appwin = dynamic_cast<ViewerAppWindow *>(w->window());
    iVertexTracer::palette.RedF    = plottingPalettes[(int) appwin->mnPalette[0]->value()];
    iVertexTracer::palette.GreenF  = plottingPalettes[(int) appwin->mnPalette[1]->value()];
    iVertexTracer::palette.BlueF   = plottingPalettes[(int) appwin->mnPalette[2]->value()];
    appwin->glWin->invalidate();
    appwin->glWin->redraw();
}

void
ViewerAppWindow::cuts_changed(Fl_Widget * w, void * userdata) {
    ViewerAppWindow * appwin = (ViewerAppWindow*) userdata;

    if( appwin->sldrLowerCut->value() >= appwin->sldrUpperCut->value() ) {
        appwin->sldrUpperCut->value(
                appwin->sldrLowerCut->value() + 1
            );
    }

    appwin->cCut->bgn = appwin->sldrLowerCut->value();
    appwin->cCut->end = appwin->sldrUpperCut->value();

    appwin->glWin->invalidate();
    appwin->glWin->redraw();
}

void
ViewerAppWindow::show_inspector(Fl_Widget * w, void * userdata) {
    ViewerAppWindow * appwin = (ViewerAppWindow*) userdata;
    if(!appwin->wInspector) {
        appwin->wInspector = new ProcessorUIInspector(appwin);
        appwin->wInspector->show();
    } else {
        if( appwin->lbtnInspector->value() ) {
            appwin->wInspector->show();
        } else {
            appwin->wInspector->hide();
        }
    }
}

void
ViewerAppWindow::change_cutting_var_to( Fl_Widget * w, void * userdata ) {
    ViewerAppWindow * appwin = dynamic_cast<ViewerAppWindow *>(w->window());
    if( !appwin ) {
        hraise(badArchitect, "bad type provided");
    }
    appwin->get_glWin().get_tracer().invalidate_range();
    appwin->switch_variable( (iVertexTracer::CuttingConfiguration*) userdata );
}

void
ViewerAppWindow::plot_slice(Fl_Widget *w, void * userdata) {
    eprintf( "Routine unsupported. Sorry.\n" );
}

void
ViewerAppWindow::take_picture(Fl_Widget *w, void * userdata) {
    ViewerAppWindow * appwin = dynamic_cast<ViewerAppWindow *>(w->window());
    assert( appwin->imgRuler->h() == appwin->glWin->h() );
    UShort imgWidth   = appwin->imgRuler->w() + appwin->glWin->w(),
           imgHeight  = appwin->glWin->h();
    if( !appwin ) {
        hraise(badArchitect, "bad type provided");
    }
    
    hph_RGB * pic = new hph_RGB[ imgHeight*imgWidth ];
    png_bytep * rowPtrs = new png_bytep[imgHeight];
    for( UShort i = 0; i < imgHeight; ++i ) {
        rowPtrs[i] = (png_bytep) (
                pic + (imgHeight-i-1)*imgWidth);
    }
    {
        hph_RGB * glPic = new hph_RGB[ imgHeight*appwin->glWin->w() ];
        glReadPixels(
                0, 0,
                appwin->glWin->w(), appwin->glWin->h(),
                GL_RGB, GL_UNSIGNED_BYTE,
                glPic
            );
        UByte * cR = fl_read_image( null,
                appwin->imgRuler->x(), appwin->imgRuler->y(),
                appwin->imgRuler->w(), appwin->imgRuler->h(), 0 );
        for( UShort i = 0; i < imgHeight; ++i ) {
            memcpy( pic + i*imgWidth, cR + (imgHeight-i-1)*appwin->imgRuler->w()*3,
                    sizeof(hph_RGB)*(appwin->imgRuler->w()) );
            memcpy( pic + i*imgWidth + appwin->imgRuler->w(),
                    glPic + i*(appwin->glWin->w()),
                    sizeof(hph_RGB)*(appwin->glWin->w()) );
        }
        delete [] cR;
        delete glPic;
    }

    std::string fname;
    if(!userdata) {
        fname = std::string( get_timestamp() );
        boost::replace_all(fname, "/", "");
        fname = "/dev/shm/" + fname + ".png";
    } else {
        fname = std::string( reinterpret_cast<const char*>(userdata) );
    }

    FILE *fp = fopen(fname.c_str(), "wb");
    png_structp png_ptr = png_create_write_struct(
                              PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    if(setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during init_io.\n");
    } png_init_io(png_ptr, fp);

    if (setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during writing header.\n");
    } png_set_IHDR(png_ptr, info_ptr, imgWidth, imgHeight,
                 /* bit_depth  */ 8,
                 /* color_type */ PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
      //png_set_text( png_ptr, info_ptr, text, num_text );
      png_write_info(png_ptr, info_ptr);
    if (setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during writing bytes.\n");
    } png_write_image(png_ptr, rowPtrs);

    if(setjmp(png_jmpbuf(png_ptr))) {
        hraise(thirdParty, "LibPNG: Error during end of write.\n");
    } png_write_end(png_ptr, NULL);
    fclose(fp);
    png_destroy_info_struct( png_ptr, &info_ptr );
    delete pic;
}

void
ViewerAppWindow::take_video(Fl_Widget *w, void * userdata) {
    ViewerAppWindow * appwin = dynamic_cast<ViewerAppWindow *>(w->window());
    if( !appwin ) {
        hraise(badArchitect, "bad type provided");
    }
    std::string outDir = std::string(
                    "/dev/shm/ifw-pde-video/"
                    /*reinterpret_cast<const char*>(userdata)*/ );
    if( !fs::is_directory(outDir) ) {
        fs::create_directories( outDir );
    }
    ViewerAppWindow & win = dynamic_cast<ViewerAppWindow &>(
                        *dynamic_cast<Fl_Button&>(*w).window()
                );
    Size nFrame = 0;
    for( auto it = win.sldrNFrame->_allowedFrames.begin();
              it != win.sldrNFrame->_allowedFrames.end(); ++it, ++nFrame){
        win.sldrNFrame->value( *it );
        FrameSlider::change( win.sldrNFrame, null );
        win.get_glWin().damage(FL_DAMAGE_ALL);
        Fl::flush();
        {
            char bf[32];
            snprintf( bf, 16, "frame-%08zu", nFrame );
            std::string fname = std::string( bf );
            boost::replace_all(fname, "/", "");
            fname = outDir + fname + ".png";
            take_picture(win.btnTakePic, (void*) fname.c_str());
        }
    }
            // TODO: make it configurable
            char bf2[256];
            std::string fname( get_timestamp() );
            boost::replace_all(fname, "/", "");
            snprintf( bf2, 256, "ffmpeg -r 10 -f image2 -i %sframe-%%08d.png \
-vcodec mpeg4 -b:v 800k %s%s.avi", outDir.c_str(), outDir.c_str(), fname.c_str() );
            std::cout << bf2 << std::endl;
            int res = system( bf2 );
            if(res) { warning( "system command failure.\n" ); }
            snprintf( bf2, 256, "rm -rf %s/frame-*.png", outDir.c_str() );
            res = system( bf2 );
            if(res) { warning( "system command failure.\n" ); }

            snprintf( bf2, 256, "%s/%s.cfg", outDir.c_str(), fname.c_str() );
            std::ofstream outCfg( bf2 );
            //outCfg << Application::self.co();  //TODO
            outCfg.close();
}

void
ViewerAppWindow::change_displaying_val_to(Fl_Widget * w, void * userdata) {
    // TODO: poor design -- pointer-to-int casts
    GLVertexTracer::CValueType ct =
            (GLVertexTracer::CValueType) ((size_t) (userdata));
    ViewerAppWindow * appwin = dynamic_cast<ViewerAppWindow *>(w->window());
    appwin->glWin->get_tracer().value_type( ct );
    // TODO: ...redraw?
}

void
ViewerAppWindow::change_vtracer_settings(Fl_Widget * w, void * userdata) {
    ViewerAppWindow * appwin = dynamic_cast<ViewerAppWindow *>(w->window());
    UByte flags = 0;
    if( appwin->btnDrawBounds->value() ) {
        flags |= GLVertexTracer::DRAW_EDGES;
    }
    if( appwin->btnDrawWareframe->value() ) {
        flags |= GLVertexTracer::DRAW_WIREFRAME;
    }
    if( appwin->btnDrawGrid->value() ) {
        flags |= GLVertexTracer::DRAW_GRID;
    }
    if( appwin->btnDrawInfo->value() ) {
        flags |= GLVertexTracer::PRINT_INFO;
    }
    if( appwin->btnMarkVortexCenter->value() ) {
        flags |= GLVertexTracer::MARK_VORTICES;
    }
    appwin->glWin->get_tracer().drawing_flags( flags );
    appwin->glWin->get_tracer().vecscale( appwin->snrVectorLength->value() );
}

void
ViewerAppWindow::_refresh_cut_sliders() {
    sldrLowerCut->bounds( cCut->min,     cCut->max - 1);
    sldrLowerCut->value(  cCut->bgn );
    sldrUpperCut->bounds( cCut->min + 1, cCut->max );
    sldrUpperCut->value(  cCut->end );
}

ViewerAppWindow::ViewerAppWindow( Application * app_ ) :
            # ifndef NDOUBLE_WINDOW
            Fl_Double_Window(
                app_->co()["Viewer.viewport-w"].as<UShort>() + waWidth + 60,
                app_->co()["Viewer.viewport-h"].as<UShort>() + 65,
                "PDE Sol view" ),
            # else
            Fl_Window(
                app_->co()["Viewer.viewport-w"].as<UShort>() + waWidth + 60,
                app_->co()["Viewer.viewport-h"].as<UShort>() + 65,
                "PDE Sol view" ),
            # endif
            wInspector(nullptr),
            _app(app_),
            app(*_app),
            cDrawer( ViewerAppWindow::_drawing_draw_map ) {
    const ifw::InitObj & io = app.co();
    const ifw::iEqu & P = app.processor();
    iVertexTracer::cuts = new iVertexTracer::CuttingConfiguration [P.n_dims()];
    //Fl::visual(FL_DOUBLE|FL_INDEX);
    Fl::gl_visual(FL_RGB);
    cCut = iVertexTracer::cuts;

    for( UByte d = 0; d < P.n_dims(); ++d ) {
        iVertexTracer::cuts[d].bgn = iVertexTracer::cuts[d].min = 0;
        iVertexTracer::cuts[d].end = iVertexTracer::cuts[d].max = P.n_nodes(d) - 1;
    }
    UShort vOffset = 10;
    const UShort hOffset = io["Viewer.viewport-w"].as<UShort>();
    // OpenGL window
    glWin = new OGLViewport( io, P );
    { // lower cut slider
        sldrLowerCut = new Fl_Value_Slider(
                    hOffset + 60, vOffset,
                    waWidth*2/3, 20, "lower");
        sldrLowerCut->align(FL_ALIGN_RIGHT);
        sldrLowerCut->type(FL_HOR_NICE_SLIDER);
        sldrLowerCut->bounds(0, 1);
        sldrLowerCut->value(1);
        sldrLowerCut->step(1);
        sldrLowerCut->callback(cuts_changed, (void*)this);
        vOffset += 25;
    }
    {// upper cut slider
        sldrUpperCut = new Fl_Value_Slider(
                    hOffset + 60, vOffset,
                    waWidth*2/3, 20, "upper");
        sldrUpperCut->align(FL_ALIGN_RIGHT);
        sldrUpperCut->type(FL_HOR_NICE_SLIDER);
        sldrUpperCut->bounds(0, 1);
        sldrUpperCut->value(0);
        sldrUpperCut->step(1);
        sldrUpperCut->callback(cuts_changed, (void*)this);
        vOffset += 30;
    }
    {// cut variable switcher
        mnVarSwitcher = new Fl_Menu_Bar(hOffset+60,vOffset,waWidth/2 - 10,25);
        mnVarSwitcher->add("switch var/X1", 0, change_cutting_var_to, iVertexTracer::cuts  , FL_MENU_RADIO | FL_MENU_VALUE);
        for( UByte d = 1; d < P.n_dims(); ++d ) {
            char bf[32];
            snprintf( bf, 32, "switch var/X%d", (int) (d+1) );
            mnVarSwitcher->add(bf, 0, change_cutting_var_to, iVertexTracer::cuts+d, FL_MENU_RADIO);
        }
    }
    // slice plotter button
    btnPlotSlice = new Fl_Button( hOffset + 60 + waWidth/2, vOffset,
                                  waWidth/2 - 10, 25, "plot slice" );
    btnPlotSlice->callback(plot_slice);
    vOffset += 35;

    {// displayed value switcher
        mnValSwitcher = new Fl_Menu_Bar(hOffset+60,vOffset,waWidth/2 - 10,25);         //TODO
        mnValSwitcher->add("switch val/order parameter",    0, change_displaying_val_to, (void*) GLVertexTracer::orderParameter,            FL_MENU_RADIO | FL_MENU_VALUE);
        mnValSwitcher->add("switch val/order p. (real)",    0, change_displaying_val_to, (void*) GLVertexTracer::orderParameterReal,        FL_MENU_RADIO);
        mnValSwitcher->add("switch val/order p. (imag)",    0, change_displaying_val_to, (void*) GLVertexTracer::orderParameterImaginary,   FL_MENU_RADIO);
        mnValSwitcher->add("switch val/vect. p. (scalar)",  0, change_displaying_val_to, (void*) GLVertexTracer::vectorPotentialNormalC,    FL_MENU_RADIO);
        mnValSwitcher->add("switch val/vector potential",   0, change_displaying_val_to, (void*) GLVertexTracer::vectorPotential,           FL_MENU_RADIO);
        mnValSwitcher->add("switch val/currents",           0, change_displaying_val_to, (void*) GLVertexTracer::vectorCurrents,            FL_MENU_RADIO);
    }

    // take a shot/video buttons
    btnTakePic = new Fl_Button( hOffset + 60 + waWidth/2, vOffset,
                                    waWidth/4 - 10, 25, "pic" );
    btnTakePic->callback(take_picture, null);
    btnTakeVideo = new Fl_Button( hOffset + 60 + waWidth/2 + waWidth/4, vOffset,
                                    waWidth/4 - 10, 25, "vid" );
    btnTakeVideo->callback(take_video, null);
    vOffset += 30;
    // processor inspector button
    lbtnInspector = new Fl_Light_Button( hOffset + 60 + waWidth/2, vOffset,
                                         waWidth/2 - 10, 25, "inspector" );
    lbtnInspector->callback(show_inspector, this);

    // frame slider
    std::vector<Size> frames = app.get_frames_list();
    sldrNFrame = new FrameSlider(
                frames,
                60, io["Viewer.viewport-h"].as<UShort>() + 10,
                io["Viewer.viewport-w"].as<UShort>(), 15, null);
    sldrNFrame->callback( FrameSlider::change, this );
    _refresh_cut_sliders();

    btnDrawBounds = new Fl_Check_Button(
            10, io["Viewer.viewport-h"].as<UShort>() + 40,
            waWidth/2 - 10, 25, "bounds" );
    btnDrawBounds->callback(ViewerAppWindow::change_vtracer_settings, null);
    btnDrawWareframe = new Fl_Check_Button(
            waWidth/2 + 25, io["Viewer.viewport-h"].as<UShort>() + 40,
            waWidth/2 - 10, 25, "wareframe" );
    btnDrawWareframe->callback(ViewerAppWindow::change_vtracer_settings, null);
    btnDrawGrid = new Fl_Check_Button(
            2*(waWidth/2 + 25), io["Viewer.viewport-h"].as<UShort>() + 40,
            waWidth/2 - 10, 25, "draw grid" );
    btnDrawGrid->callback(ViewerAppWindow::change_vtracer_settings, null);
    btnDrawInfo = new Fl_Check_Button(
            3*(waWidth/2 + 25), io["Viewer.viewport-h"].as<UShort>() + 40,
            waWidth/2 - 10, 25, "print info" );
    btnDrawInfo->callback(ViewerAppWindow::change_vtracer_settings, null);
    btnMarkVortexCenter = new Fl_Check_Button(
            4*(waWidth/2 + 25), io["Viewer.viewport-h"].as<UShort>() + 40,
            waWidth/2 - 10, 25, "mark vortices" );
    btnMarkVortexCenter->callback(ViewerAppWindow::change_vtracer_settings, null);
    {
        snrVectorLength = new Fl_Spinner(
            6*(waWidth/2 + 25), io["Viewer.viewport-h"].as<UShort>() + 40,
            waWidth/2 - 10, 25, "vector len");
        snrVectorLength->type(FL_FLOAT_INPUT);
        snrVectorLength->minimum(0.);
        snrVectorLength->maximum(10.);
        snrVectorLength->step(0.01);
        snrVectorLength->value(0.021);
        snrVectorLength->format("%2.2f");
        snrVectorLength->callback(ViewerAppWindow::change_vtracer_settings, null);
    }
    
    {
        static const char _labels[][2] = { "R", "G", "B" };
        for( UByte c = 0; c < 3; ++c) {
            mnPalette[c] = new Fl_Spinner(
                7*(waWidth/2 + 25) + c*75, io["Viewer.viewport-h"].as<UShort>() + 40,
                50,25, _labels[c]);
            mnPalette[c]->minimum(0);
            mnPalette[c]->maximum(36);
            mnPalette[c]->callback( ViewerAppWindow::change_palette );
        }
        mnPalette[0]->value( io["Viewer.paletteR"].as<UShort>() );
        mnPalette[1]->value( io["Viewer.paletteG"].as<UShort>() );
        mnPalette[2]->value( io["Viewer.paletteB"].as<UShort>() );
    }


    // Color ruler
    imgRuler = new ColorRuler( 5, 5, 45, io["Viewer.viewport-h"].as<UShort>() );

    end();
}

void
ViewerAppWindow::_drawing_draw_map(  const ifw::iEqu& P, GLVertexTracer& t ) {
    // TODO: just refilling textures doesn't work.
    // Drastically slows down application.
    t.init_textures();
    t.draw();
}

void
ViewerAppWindow::_drawing_draw_vfield(  const ifw::iEqu& P, GLVertexTracer& t ) {
    t.draw();
}

int
run_viewer( Application * app ) {
     ViewerAppWindow win( app );
     win.show();
     return(Fl::run());
}
# endif
# endif

