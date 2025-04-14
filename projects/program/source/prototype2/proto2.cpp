#include "proto2.hpp"
#include <threads.h>
#include <glbinding/gl/gl.h>
#include <awc2/C/awc2.h>
#include "gfx2.hpp"


int prototype_splatting_3d_texture2()
{    
    const struct timespec pause_sleep_duration{
        .tv_sec = 0,
        .tv_nsec = 6944444
    };
    proto2::GraphicsContext glctx;
    u8 contextid{0};
    u8 alive{true}, paused{false};

    awc2init();
    contextid = awc2createContext();
    AWC2WindowDescriptor defwindesc;
    awc2WindowDescriptorDefault(&defwindesc);
    AWC2ContextDescriptor ctxtinfo = {
        contextid,
        {0},
        1920,
        1080,
        defwindesc
    };
    awc2initializeContext(&ctxtinfo);
    
    
    awc2setCurrentContext(contextid);
    glctx.initialize();


    while(alive) 
    {
        awc2newframe();
        awc2begin();

        gl::glClear(gl::GL_COLOR_BUFFER_BIT | gl::GL_DEPTH_BUFFER_BIT);
        if(!paused) {
            glctx.update();
        } else {
            thrd_sleep(&pause_sleep_duration, NULL);
        }
        alive   = !awc2getContextStatus(contextid) && !awc2isKeyPressed(AWC2_KEYCODE_ESCAPE);
        paused ^= awc2isKeyPressed(AWC2_KEYCODE_P);
        awc2end();
    }


    glctx.destroy();
    awc2destroyContext(contextid);
    awc2destroy();
    return 1;
}