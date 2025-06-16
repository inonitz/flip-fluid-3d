#pragma once
#include <util/vec2.hpp>
#include "gl/shader2.hpp"
#include "gl/texturearray.hpp"
#include "gl/persistentbuf.hpp"


namespace proto2 {


struct PressureSolver {
public:
    void init(
        util::math::vec3u const& grid_size, 
        f32 dx, 
        u32 operationsPerThread
    );
    __force_inline void destroy()
    {
        destroyShaders();
        destroyTextures();
        m_l2normBuf.destroy();
        return;
    }


    u32 solve(
        u32 staggeredu_texid,
        u32 staggeredv_texid,
        u32 staggeredw_texid,
        f32 density_div_dt,
        u32 celltypes_texid
    );


private:
    void initShaders();
    void initTextures();
    
    __force_inline void destroyShaders() {
        for(auto& prog : m_ops) {
            prog.destroy();
        }
        return;
    }
    __force_inline void destroyTextures() {
        m_registers.destroy();
        return;
    }

    void createBMatrix(
        u32 staggeredVelocityU, 
        u32 staggeredVelocityV, 
        u32 staggeredVelocityW,
        f32 density_div_dt,
        u32 bmatrix_texid
    );
    void createImplicitA(
        u32 celltype_tex,
        u32 imp_a_texid
    ); 
    f32 l2norm(u32 texture);
    void copy(u32 dest_id, u32 src_id);
    void dotprod(
        u32 vec_texidA,
        u32 vec_texidB,
        u32 float_texidC
    );
    void applyA(
        u32 imp_a_texid,
        u32 vec_texinId,
        u32 vec_texoutId
    );
    void update_x_and_r(
        u32 rprev_texid,
        u32 xprev_texid,
        u32 alpha_numer_texid,
        u32 alpha_denom_texid,
        u32 pprev_texid,
        u32 applyApprev_texid,
        u32 rnext_imgid,
        u32 xnext_imgid
    );
    void update_p(
        u32 pprev_texid,
        u32 rnext_texid,
        u32 beta_numer_texid,
        u32 beta_denom_texid,
        u32 pnext_imgid
    );
    void waitForResultVisibillity();


private:
    void* m_fence;
    PersistentBuffer       m_l2normBuf;
    TextureDescriptorArray m_registers;
    std::array<u32, 15>    m_registerRef;
    std::array<ComputeShader,     8> m_ops;
    std::array<util::math::vec3u, 8> m_localWorkgroupSize;
    std::array<util::math::vec3u, 8> m_dispatchSize;
    util::math::vec3u m_gridSize;
    u32               m_reductionFactor;
    f32               m_dl;


    ComputeShader& mr_implicitA  = m_ops[0];
    ComputeShader& mr_set_b_udiv = m_ops[1];
    ComputeShader& mr_copy       = m_ops[2];
    ComputeShader& mr_l2norm     = m_ops[3];
    ComputeShader& mr_dot_prod   = m_ops[4];
    ComputeShader& mr_applya     = m_ops[5];
    ComputeShader& mr_x_r_update = m_ops[6];
    ComputeShader& mr_p_update   = m_ops[7];


    u32& mr_rprev    = m_registerRef[0];
    u32& mr_rnext    = m_registerRef[1];
    u32& mr_pprev    = m_registerRef[2];
    u32& mr_pnext    = m_registerRef[3];
    u32& mr_xprev    = m_registerRef[4];
    u32& mr_xnext    = m_registerRef[5];
    u32& mr_ATexture = m_registerRef[6];
    u32& mr_vectorRegTex0 = m_registerRef[7];
    u32& mr_vectorRegTex1 = m_registerRef[8];
    u32& mr_vectorRegTex2 = m_registerRef[9];
    u32& mr_vectorRegTex3 = m_registerRef[10];
    u32& mr_intRegTex0 = m_registerRef[11];
    u32& mr_intRegTex1 = m_registerRef[12];
    u32& mr_intRegTex2 = m_registerRef[13];
    u32& mr_intRegTex3 = m_registerRef[14];
};


} /* namespace proto2 */