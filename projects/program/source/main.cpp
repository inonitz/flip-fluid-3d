#include <util/macro.h>
#include <util/base_type.h>
#include <util/marker2.hpp>


#ifndef _SELECT_MAIN
#   define _SELECT_MAIN 3
#endif

#if _SELECT_MAIN == -1
#   include "imgui_demo/render_demo.hpp"
#elif _SELECT_MAIN == 0
#   include "29cleanup3/improve.hpp"
#elif _SELECT_MAIN == 1
#   include "prototype0/proto.hpp"
#elif _SELECT_MAIN == 2
#   include "prototype1/proto1.hpp"
#elif _SELECT_MAIN == 3
#   include "prototype2/proto2.hpp"
#elif _SELECT_MAIN == 4
#   include "conjgrad/cg.hpp"
#endif



int main(__unused int argc, __unused char* argv[]) {
    i32 out = 0x69;
    markstr("Successful main enter");

#if _SELECT_MAIN == -1
    out = render_imgui_demo_window();
#elif _SELECT_MAIN == 0
    out = cleanup329::gpugems38_demo_last();
#elif _SELECT_MAIN == 1
    out = prototype_3d_scene();
#elif _SELECT_MAIN == 2
    out = prototype_splatting_3d_texture();
#elif _SELECT_MAIN == 3
    out = prototype_splatting_3d_texture2();
#elif _SELECT_MAIN == 4
    out = prototype_specialized_conjugate_gradient_algorithm();
#endif


    markstr("Successful Exit");
    return out;
}