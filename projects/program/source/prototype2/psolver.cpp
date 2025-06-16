#include "psolver.hpp"
#include <util/util.hpp>
#include <glbinding/gl/gl.h>


namespace proto2 {


void PressureSolver::init(
    util::math::vec3u const& grid_size, 
    f32 dx,
    u32 operationsPerThread
) {
    m_gridSize        = grid_size;
    m_reductionFactor = operationsPerThread;
    m_dl = dx;
    initShaders();
    initTextures();


    m_l2normBuf.create(4, 1, gl::GL_MAP_READ_BIT);
    return;
}


u32 PressureSolver::solve(
    u32 staggeredu_texid,
    u32 staggeredv_texid,
    u32 staggeredw_texid,
    f32 density_div_dt,
    u32 celltypes_texid
) {

    createBMatrix(staggeredu_texid, staggeredv_texid, staggeredw_texid, density_div_dt,
        mr_vectorRegTex0
    );
    copy(mr_rprev, mr_vectorRegTex0);
    copy(mr_pprev, mr_vectorRegTex0);
    createImplicitA(celltypes_texid, mr_ATexture);


    f32 r_2norm = 1.0f, epsilon = 10-6.0f;
    i32 iter_max = 1000;
    while(iter_max > 0 && r_2norm > epsilon) 
    {
        applyA(mr_ATexture, mr_pprev,         mr_vectorRegTex1);
        dotprod(mr_rprev, mr_rprev,           mr_intRegTex0);
        waitForResultVisibillity(); /* for mr_vectorRegTex[1] */
        dotprod(mr_pprev, mr_vectorRegTex1, mr_intRegTex1);
        waitForResultVisibillity(); /* for mr_intRegTex0 &  mr_intRegTex1 */
        update_x_and_r(mr_rprev, mr_xprev,
            mr_intRegTex0,
            mr_intRegTex1,
            mr_pprev,
            mr_vectorRegTex1,
            mr_rnext,
            mr_xnext
        );
        waitForResultVisibillity(); /* for mr_rnext */
        dotprod(mr_rnext, mr_rnext, mr_intRegTex2);

        waitForResultVisibillity(); /* for mr_intRegTex2 */
        update_p(mr_pprev, mr_rnext,
            mr_intRegTex2,
            mr_intRegTex0,
            mr_pnext
        );


        r_2norm = l2norm(mr_rnext);
        std::swap(mr_rprev, mr_rnext);
        std::swap(mr_pprev, mr_pnext);
        std::swap(mr_xprev, mr_xnext);
    }


    return m_registers[mr_xprev].id();
}


void PressureSolver::initShaders()
{
    const std::array<ShaderData, 8> computeFiles = {{
        { "projects/program/source/prototype2/shaders/solver/cg/implicitA.comp",  __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/cg/set_b_udiv.comp", __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/cg/copy.comp",       __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/cg/l2norm.comp",     __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/cg/dot_prod.comp",   __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/cg/applya.comp",     __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/cg/x_r_update.comp", __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/cg/p_update.comp",   __scast(u32, gl::GL_COMPUTE_SHADER)},
    }};
    constexpr std::array<bool, 8> usesReductionFactor = {
        false,
        false,
        false,
        true,
        true,
        false,
        false,
        false
    };
    const auto localSize = util::math::vec3u{ 32, 2, 1 };
    
    
    util::__memset(m_localWorkgroupSize.data(), m_localWorkgroupSize.size(), localSize);
    

    u32 localReductionFactor = m_reductionFactor;
    for(u32 i = 0; i < m_ops.size(); ++i) {
        localReductionFactor = (usesReductionFactor[i]) ? m_reductionFactor : 1;
        m_ops[i].load(computeFiles[i]);
        m_ops[i].resizeLocal(m_localWorkgroupSize[i].begin());
        m_ops[i].resizeDispatchWithProblemSize(m_gridSize.begin(), localReductionFactor);
    }
    for(u32 i = 0; i < m_ops.size(); ++i) {
        if(!m_ops[i].compile())
            ifcrash(true);
        m_ops[i].info();
    }
    return;
}


void PressureSolver::initTextures()
{
    const std::array<TextureDescriptorArray::TextureFormat, 15> texFormats = {
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_rprev */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_rnext */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_pprev */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_pnext */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_xprev */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_xnext */

        TextureDescriptorArray::TextureFormat{ gl::GL_R16UI, gl::GL_RED_INTEGER, gl::GL_UNSIGNED_SHORT }, /* mr_ATexture */

        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_vectorRegTex0 */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_vectorRegTex1 */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_vectorRegTex2 */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F, gl::GL_RED, gl::GL_FLOAT }, /* mr_vectorRegTex3 */

        TextureDescriptorArray::TextureFormat{ gl::GL_R32I,  gl::GL_RED_INTEGER, gl::GL_INT }, /* mr_intRegTex0 */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32I,  gl::GL_RED_INTEGER, gl::GL_INT }, /* mr_intRegTex1 */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32I,  gl::GL_RED_INTEGER, gl::GL_INT }, /* mr_intRegTex2 */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32I,  gl::GL_RED_INTEGER, gl::GL_INT }  /* mr_intRegTex3 */
    };
    const std::array<util::math::vec3u, 15> texSizes = {
        m_gridSize, /* mr_rprev */
        m_gridSize, /* mr_rnext */
        m_gridSize, /* mr_pprev */
        m_gridSize, /* mr_pnext */
        m_gridSize, /* mr_xprev */
        m_gridSize, /* mr_xnext */

        m_gridSize, /* mr_ATexture */
        
        m_gridSize, /* mr_vectorRegTex0 */
        m_gridSize, /* mr_vectorRegTex1 */
        m_gridSize, /* mr_vectorRegTex2 */
        m_gridSize, /* mr_vectorRegTex3 */

        { 1, 1, 1 }, /* mr_intRegTex0 */
        { 1, 1, 1 }, /* mr_intRegTex1 */
        { 1, 1, 1 }, /* mr_intRegTex2 */
        { 1, 1, 1 }  /* mr_intRegTex3 */
    };


    m_registers.create(15);
    for(u32 i = 0; i < m_registers.size(); ++i) {
        m_registers[i].initialize(texFormats[i], texSizes[i]);
        m_registerRef[i] = i;
    }


    return;
}


void PressureSolver::createBMatrix(
    u32 stagu, u32 stagv, u32 stagw,
    f32 density_div_dt,
    u32 bmatrix_texid
) {
    mr_set_b_udiv.bind();
    mr_set_b_udiv.uniform1i("staggeredu", 0);
    mr_set_b_udiv.uniform1i("staggeredv", 1);
    mr_set_b_udiv.uniform1i("staggeredw", 2);
    mr_set_b_udiv.uniform1i("initialBMatrix", 3);
    mr_set_b_udiv.uniform1f("ku_neg_pdlsq_dt", m_dl * m_dl * density_div_dt);
    mr_set_b_udiv.uniform1f("ku_invdl", m_dl);
    gl::glBindTextureUnit(0, stagu);
    gl::glBindTextureUnit(1, stagv);
    gl::glBindTextureUnit(2, stagw);
    m_registers[bmatrix_texid].bindImage(3, gl::GL_WRITE_ONLY);
    mr_set_b_udiv.dispatch();
    return;
}


void PressureSolver::createImplicitA(
    u32 celltype_tex,
    u32 imp_a_texid
) {
    mr_implicitA.bind();
    mr_implicitA.uniform1i("celltypes",          0);
    mr_implicitA.uniform1i("implicitABitfields", 1);
    gl::glBindTextureUnit(0, celltype_tex);
    m_registers[imp_a_texid].bindImage(1, gl::GL_WRITE_ONLY);
    mr_implicitA.dispatch();
    return;
}


    
f32 PressureSolver::l2norm(u32 tex) 
{
    auto jamAFenceToWaitForGPUBufferVisibility = [this]() {
        if(m_fence) {
            gl::glDeleteSync(__rcast(gl::__GLsync*, m_fence));
        }
        m_fence = gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        return;
    };


    mr_l2norm.bind();
    mr_l2norm.uniform1i("vecA",   0);
    mr_l2norm.uniform1i("floatB", 1);
    mr_l2norm.uniform1ui("ku_reductionFactor", m_reductionFactor);
    mr_l2norm.uniform3uiv("ku_gridsize",       m_gridSize.begin());
    gl::glBindTextureUnit(0, tex);
    mr_l2norm.StorageBlock("floatB", 1);
    mr_l2norm.dispatch();
    gl::glMemoryBarrier(gl::GL_ALL_BARRIER_BITS);
    jamAFenceToWaitForGPUBufferVisibility();


    return __rcast(const f32*, m_l2normBuf.begin() )[0];
}


void PressureSolver::copy(u32 dest_id, u32 src_id)
{
    mr_copy.bind();
    mr_copy.uniform1i("to_copy", 0);
    mr_copy.uniform1i("dest",    1);
    m_registers[src_id].bindUnit(0);
    m_registers[dest_id].bindImage(1, gl::GL_WRITE_ONLY);
    mr_copy.dispatch();
    return;
}


void PressureSolver::dotprod(
    u32 vec_texidA,
    u32 vec_texidB,
    u32 float_texidC
) {
    mr_dot_prod.bind();
    mr_dot_prod.uniform1i("vecA", 0);
    mr_dot_prod.uniform1i("vecB", 1);
    mr_dot_prod.uniform1i("floatC", 2);
    mr_dot_prod.uniform1ui("ku_reductionFactor", m_reductionFactor);
    mr_dot_prod.uniform3uiv("ku_gridsize",       m_gridSize.begin());
    m_registers[vec_texidA].bindUnit(0);
    m_registers[vec_texidB].bindUnit(1);
    m_registers[float_texidC].bindImage(2, gl::GL_WRITE_ONLY);
    mr_dot_prod.dispatch();
    return;
}


void PressureSolver::applyA(
    u32 imp_a_texid,
    u32 vec_texinId,
    u32 vec_texoutId
) {
    mr_applya.bind();
    mr_applya.uniform1i("implicitA", 0);
    mr_applya.uniform1i("vecIn",     1);
    mr_applya.uniform1i("vecOut",    2);
    m_registers[imp_a_texid].bindUnit(0);
    m_registers[vec_texinId].bindUnit(1);
    m_registers[vec_texoutId].bindImage(2, gl::GL_WRITE_ONLY);
    mr_applya.dispatch();
    return;
}


void PressureSolver::update_x_and_r(
    u32 rprev_texid,
    u32 xprev_texid,
    u32 alpha_numer_texid,
    u32 alpha_denom_texid,
    u32 pprev_texid,
    u32 applyApprev_texid,
    u32 rnext_imgid,
    u32 xnext_imgid
) {
    mr_x_r_update.bind();
    mr_x_r_update.uniform1i("r_prev",        0);
    mr_x_r_update.uniform1i("x_prev",        1);
    mr_x_r_update.uniform1i("alpha_numer",   2);
    mr_x_r_update.uniform1i("alpha_denom",   3);
    mr_x_r_update.uniform1i("p_prev",        4);
    mr_x_r_update.uniform1i("applyA_p_prev", 5);
    mr_x_r_update.uniform1i("r_next",        6);
    mr_x_r_update.uniform1i("x_next",        7);
    m_registers[rprev_texid      ].bindUnit(0);
    m_registers[xprev_texid      ].bindUnit(1);
    m_registers[alpha_numer_texid].bindUnit(2);
    m_registers[alpha_denom_texid].bindUnit(3);
    m_registers[pprev_texid      ].bindUnit(4);
    m_registers[applyApprev_texid].bindUnit(5);
    m_registers[rnext_imgid].bindImage(6, gl::GL_WRITE_ONLY);
    m_registers[xnext_imgid].bindImage(7, gl::GL_WRITE_ONLY);
    mr_x_r_update.dispatch();
    return;
}


void PressureSolver::update_p(
    u32 pprev_texid,
    u32 rnext_texid,
    u32 beta_numer_texid,
    u32 beta_denom_texid,
    u32 pnext_imgid
) {
    mr_p_update.bind();
    mr_p_update.uniform1i("p_prev",     0);
    mr_p_update.uniform1i("r_next",     1);
    mr_p_update.uniform1i("beta_numer", 2);
    mr_p_update.uniform1i("beta_denom", 3);
    mr_p_update.uniform1i("p_next",     4);
    m_registers[pprev_texid     ].bindUnit(0);
    m_registers[rnext_texid     ].bindUnit(1);
    m_registers[beta_numer_texid].bindUnit(2);
    m_registers[beta_denom_texid].bindUnit(3);
    m_registers[pnext_imgid].bindImage(4, gl::GL_WRITE_ONLY);
    mr_p_update.dispatch();
    return;
}


void PressureSolver::waitForResultVisibillity() {
    gl::glMemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | gl::GL_TEXTURE_FETCH_BARRIER_BIT);
    return;
}


} /* namespace proto2 */