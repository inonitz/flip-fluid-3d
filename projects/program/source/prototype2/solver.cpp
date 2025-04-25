#include "solver.hpp"
#include <algorithm>
#include <glbinding/gl/gl.h>
#include <util/random.hpp>
#include <util/marker2.hpp>


#define TIME_CODE_BLOCK_CUSTOM(enable_gpu_time, counter_cpu, counter_gpu, ...) \
    if (boolean(enable_gpu_time)) \
    { \
        /* No need to mention the GPU-timer overhead on the cpu-side */ \
        /* but i assume its negligeble */ \
        \
        counter_cpu.begin(); \
        Time::GPUTimer::begin(counter_gpu); \
        __VA_ARGS__; \
        Time::GPUTimer::end(counter_gpu); \
        counter_cpu.end(); \
    } else { \
        TIME_NAMESPACE_TIME_CODE_BLOCK(counter_cpu, __VA_ARGS__); \
    } \




namespace proto2 {


static constexpr f32 MATERIAL_TYPE_FLUID = 2.0f;
static constexpr f32 MATERIAL_TYPE_SOLID = 1.0f;
static constexpr f32 MATERIAL_TYPE_EMPTY = 0.0f;


void FluidSolver::create()
{
    constexpr u32 particleCountX = 100;
    constexpr u32 particleCountY = 100;
    constexpr u32 particleCountZ = 100;
    m_gridSize        = { 128, 128, 128 };
    m_reductionFactor = 256;
    m_unitGridSize  = 1.0f;
    m_particleCount = { particleCountX, particleCountY, particleCountZ };
    m_dt            = 0.08f;
    // m_dt = 0.001f;
    m_gravity       = -9.8f;
    

    for(auto& gpu_timer : m_timers.m_gputimerBuf) {
        gpu_timer.create();
    }
    m_pressureSolver.init(m_gridSize, m_unitGridSize, m_reductionFactor);
    initShaders();
    initTextures();
    return;
}


void FluidSolver::destroy()
{
    destroyTextures();
    destroyShaders();
    m_pressureSolver.destroy();
    return;
}


void FluidSolver::iterateOnce()
{
    TIME_CODE_BLOCK_CUSTOM(true, m_timers.mr_advectTimeCPU        , m_timers.mr_advectTimeGPU,         advect());
    // TIME_CODE_BLOCK_CUSTOM(true, m_timers.mr_resetCellTypesTimeCPU, m_timers.mr_resetCellTypesTimeGPU, resetCellTypes());
    // TIME_CODE_BLOCK_CUSTOM(true, m_timers.mr_particleToGridTimeCPU, m_timers.mr_particleToGridTimeGPU, particleToGrid());
    // TIME_CODE_BLOCK_CUSTOM(true, m_timers.mr_addExternalForcesCPU , m_timers.mr_addExternalForcesGPU,  particleToGrid());
    // TIME_CODE_BLOCK_CUSTOM(true, m_timers.mr_normalizeTimeCPU     , m_timers.mr_normalizeTimeGPU,      normalize());
    // TIME_CODE_BLOCK_CUSTOM(true, m_timers.mr_boundariesTimeCPU    , m_timers.mr_boundariesTimeGPU,     boundaries());
    // TIME_CODE_BLOCK_CUSTOM(true, m_timers.mr_pressureSolveTimeCPU , m_timers.mr_pressureSolveTimeGPU,  pressureSolve());
    // std::swap(mr_prevTexU, mr_nextTexU);
    // std::swap(mr_prevTexV, mr_nextTexV);
    // std::swap(mr_prevTexW, mr_nextTexW);
    // std::swap(mr_prevTexP, mr_nextTexP);
    // std::swap(mr_prevPos , mr_nextPos);
    return;
}



void FluidSolver::initShaders()
{
    const std::array<ShaderData, 5> computeFiles = {{
        { "projects/program/source/prototype2/shaders/solver/advect.comp",        __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/copy.comp",          __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/stagger.comp",       __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/boundarypass.comp",  __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/flip.comp",          __scast(u32, gl::GL_COMPUTE_SHADER)}
    }};
    const auto local_size = util::math::vec3u{ 32, 2, 1 };


    for(u32 i = 0; i < m_computePrograms.size(); ++i) {
        m_computePrograms[i].load(computeFiles[i]);
        m_computePrograms[i].resizeLocal(local_size.begin());
        m_computePrograms[i].resizeDispatchWithProblemSize(m_gridSize.begin(), 1);
    }
    for(u32 i = 0; i < m_computePrograms.size(); ++i) {
        if(!m_computePrograms[i].compile())
            ifcrash(true);
    }
    return;
}


void FluidSolver::initTextures()
{
    const std::array<TextureDescriptorArray::TextureFormat, 26> texFormats = {
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F,    gl::GL_RED,  gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F,    gl::GL_RED,  gl::GL_FLOAT }, 

        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 

        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT },
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT },
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT },
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }
    };
    const std::array<util::math::vec3u, 26> texSizes = {
        m_gridSize,
        m_gridSize,

        m_particleCount,
        m_particleCount,
        m_particleCount,
        m_particleCount,

        { m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        { m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        { m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        { m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        { m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        { m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        { m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        { m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        { m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        m_gridSize,
        m_gridSize,
        m_gridSize,
        m_gridSize,
        m_gridSize
    };


    m_texRegisters.create(26);
    for(u8 i = 0; i < texFormats.size(); ++i) {
        m_texRegisters[i].initialize(texFormats[i], texSizes[i]);
        m_texID[i] = i;
    }
    mr_prevTexU = mkr_uTexture0;
    mr_prevTexV = mkr_vTexture0;
    mr_prevTexW = mkr_wTexture0;
    mr_prevTexP = mkr_pTexture0;
    mr_prevPos  = mkr_positionTexture0;
    mr_nextTexU = mkr_uTexture1;
    mr_nextTexV = mkr_vTexture1;
    mr_nextTexW = mkr_wTexture1;
    mr_nextTexP = mkr_pTexture1;
    mr_nextPos  = mkr_positionTexture1;


    std::vector<util::math::vec4f> positionBuf, colourBuf;
    auto get_randvec4 = []() -> util::math::vec4f {
        return util::math::vec4f{ random32f(), random32f(), random32f(), 0.0f };
    };
    u32 i, j, k = i = j = 0;
    __unused auto get_idxvec3 = [&i, &j, &k, this]() -> util::math::vec4f {
        auto out_val = util::math::vec4f{
            __scast(f32, i) / m_particleCount.x,
            __scast(f32, j) / m_particleCount.y,
            __scast(f32, k) / m_particleCount.z,
            1.0f
        };
        ++k;
        if(k == m_particleCount.z) {
            k = 0;
            ++j;
        }
        if(j == m_particleCount.y) {
            j = 0;
            ++i;
        }
        if(i == m_particleCount.x) {
            i = 0;
        }
        return out_val;
    };


    for(i = 0; i < m_particleCount.x; ++i) {
        for(j = 0; j < m_particleCount.y; ++j) {
            for(k = 0; k < m_particleCount.z; ++k) {
                positionBuf.push_back(util::math::vec4f{
                    __scast(f32, i) / m_particleCount.x,
                    __scast(f32, j) / m_particleCount.y,
                    __scast(f32, k) / m_particleCount.z,
                    1.0f
                });
            }
        }
    }


    m_cellTypeCPUside.resize(m_gridSize.x * m_gridSize.y * m_gridSize.z);
    util::math::Tensor3View<f32> celltypes{ 
        m_cellTypeCPUside.data(), 
        __scast(i32, m_gridSize.x),
        __scast(i32, m_gridSize.y),
        __scast(i32, m_gridSize.z)
    };
    f32 material_type = MATERIAL_TYPE_EMPTY;
    for(i = 0; i < m_gridSize.x; ++i) {
        for(j = 0; j < m_gridSize.y; ++j) {
            for(k = 0; k < m_gridSize.z; ++k) 
            {
                if( 
                    (i == 0 || i == m_gridSize.x - 1)
                    ||
                    (j == 0 || j == m_gridSize.y - 1)
                    ||
                    (k == 0 || k == m_gridSize.z - 1)
                ) {
                    material_type = MATERIAL_TYPE_SOLID;
                }
                celltypes(i, j, k) = material_type;
            }
        }
    }



    colourBuf.resize(m_particleCount.x * m_particleCount.y * m_particleCount.z);
    std::generate_n(colourBuf.begin(), colourBuf.size(), get_randvec4);


    m_texRegisters[mkr_positionTexture0].set(positionBuf.data());
    m_texRegisters[mkr_colourTexture].set(colourBuf.data());
    m_texRegisters[mkr_cellTypeTexture].set(m_cellTypeCPUside.data());
    m_texRegisters[mkr_initialCellTypeTexture].set(m_cellTypeCPUside.data());
    return;
}


void FluidSolver::waitVisibillity() {
    gl::glMemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | gl::GL_TEXTURE_FETCH_BARRIER_BIT);
    return;
}




void FluidSolver::advect()
{
    mr_advect.bind();
    mr_advect.uniform1i("velu", 0);
    mr_advect.uniform1i("velv", 1);
    mr_advect.uniform1i("velw", 2);
    mr_advect.uniform1i("prevposition", 3);
    mr_advect.uniform1i("nextposition", 4);
    mr_advect.uniform1f("ku_gravity", m_gravity);
    mr_advect.uniform1f("ku_dt",      m_dt);
    mr_advect.uniform1f("ku_dl",      m_unitGridSize);
    m_texRegisters[mr_prevTexU].bindUnit(0);
    m_texRegisters[mr_prevTexV].bindUnit(1);
    m_texRegisters[mr_prevTexW].bindUnit(2);
    m_texRegisters[mr_prevPos].bindImage(3, gl::GL_READ_ONLY);
    m_texRegisters[mr_nextPos].bindImage(4, gl::GL_WRITE_ONLY);
    mr_advect.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::resetCellTypes()
{
    // m_texRegisters[mkr_cellTypeTexture].set(m_cellTypeCPUside.data());
    copy(mkr_cellTypeTexture, mkr_initialCellTypeTexture);
    m_texRegisters[mr_prevTexU].clear();
    m_texRegisters[mr_prevTexV].clear();
    m_texRegisters[mr_prevTexW].clear();
    return;
}


void FluidSolver::particleToGrid()
{
    util::math::vec3f leftmostpos{0.0f};

    mr_stagger.bind();
    mr_stagger.uniform1i("positions",      0);
    mr_stagger.uniform1i("nextcelltypes",  1);
    mr_stagger.uniform1i("prevstaggeredu", 2);
    mr_stagger.uniform1i("prevstaggeredv", 3);
    mr_stagger.uniform1i("prevstaggeredw", 4);
    mr_stagger.uniform1i("nextstaggeredu", 5);
    mr_stagger.uniform1i("nextstaggeredv", 6);
    mr_stagger.uniform1i("nextstaggeredw", 7);
    mr_stagger.uniform3fv("u_bottomLeftmostPosition", leftmostpos.begin());
    mr_stagger.uniform1f("ku_dl", m_unitGridSize);
    m_texRegisters[mr_nextPos].bindUnit(0);
    m_texRegisters[mkr_cellTypeTexture].bindUnit(1);
    m_texRegisters[mr_prevTexU].bindUnit(2);
    m_texRegisters[mr_prevTexV].bindUnit(3);
    m_texRegisters[mr_prevTexW].bindUnit(4);
    m_texRegisters[mkr_uTexture2].bindImage(5, gl::GL_WRITE_ONLY);
    m_texRegisters[mkr_vTexture2].bindImage(6, gl::GL_WRITE_ONLY);
    m_texRegisters[mkr_wTexture2].bindImage(7, gl::GL_WRITE_ONLY);
    mr_stagger.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::normalize()
{
    static constexpr f32 k_epsilon = 1e-15f;


    mr_normalizevel.bind();
    mr_normalizevel.uniform1i("prevstaggeredu", 0);
    mr_normalizevel.uniform1i("prevstaggeredv", 1);
    mr_normalizevel.uniform1i("prevstaggeredw", 2);
    mr_normalizevel.uniform1i("nextstaggeredu", 3);
    mr_normalizevel.uniform1i("nextstaggeredv", 4);
    mr_normalizevel.uniform1i("nextstaggeredw", 5);
    mr_normalizevel.uniform1f("ku_epsilon", k_epsilon);
    mr_normalizevel.uniform1ui("ku_chooseBndDir", 3);
    m_texRegisters[mkr_uTexture2].bindUnit(0);
    m_texRegisters[mkr_vTexture2].bindUnit(1);
    m_texRegisters[mkr_wTexture2].bindUnit(2);
    m_texRegisters[mkr_uTexture3].bindImage(3, gl::GL_WRITE_ONLY);
    m_texRegisters[mkr_vTexture3].bindImage(4, gl::GL_WRITE_ONLY);
    m_texRegisters[mkr_wTexture3].bindImage(5, gl::GL_WRITE_ONLY);
    mr_normalizevel.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::boundaries()
{
    static constexpr f32 k_epsilon = 1e-15f;
    static util::math::vec3i gridSizei32{ m_gridSize };

    mr_boundarypass.bind();
    mr_boundarypass.uniform1i("prevstaggeredu", 0);
    mr_boundarypass.uniform1i("prevstaggeredv", 1);
    mr_boundarypass.uniform1i("prevstaggeredw", 2);
    mr_boundarypass.uniform1i("nextstaggeredu", 3);
    mr_boundarypass.uniform1i("nextstaggeredv", 4);
    mr_boundarypass.uniform1i("nextstaggeredw", 5);
    mr_boundarypass.uniform1f("ku_epsilon", k_epsilon);
    mr_boundarypass.uniform3iv("ku_gridsize", gridSizei32.begin());


    u32 prev[3] = {
        mkr_uTexture3,
        mkr_vTexture3,
        mkr_wTexture3
    };
    u32 next[3] = {
        mkr_uTexture4,
        mkr_vTexture4,
        mkr_wTexture4
    };  
    for(u8 i = 0; i < 3; ++i) {
        mr_boundarypass.uniform1ui("ku_chooseBndDir", i);
        m_texRegisters[ prev[0] ].bindUnit(0);
        m_texRegisters[ prev[1] ].bindUnit(1);
        m_texRegisters[ prev[2] ].bindUnit(2);
        m_texRegisters[ next[0] ].bindImage(3, gl::GL_WRITE_ONLY);
        m_texRegisters[ next[1] ].bindImage(4, gl::GL_WRITE_ONLY);
        m_texRegisters[ next[2] ].bindImage(5, gl::GL_WRITE_ONLY);
        mr_boundarypass.dispatch();
        waitVisibillity();
        std::swap(prev, next);
    }
    return;
}


void FluidSolver::pressureSolve()
{ 
    m_pressureSolver.solve(
        m_texRegisters[mkr_uTexture4].id(),
        m_texRegisters[mkr_vTexture4].id(),
        m_texRegisters[mkr_wTexture4].id(),
        1.0f / m_dt,
        m_texRegisters[mkr_cellTypeTexture].id()
    );
    return;
}


void FluidSolver::copy(u32 dest_id, u32 src_id)
{
    mr_copy.bind();
    mr_copy.uniform1i("to_copy", 0);
    mr_copy.uniform1i("dest",    1);
    m_texRegisters[src_id].bindUnit(0);
    m_texRegisters[dest_id].bindImage(1, gl::GL_WRITE_ONLY);
    mr_copy.dispatch();
    return;
}


// void FluidSolver::computepass()
// {

// }



















} /* namespace proto2 */