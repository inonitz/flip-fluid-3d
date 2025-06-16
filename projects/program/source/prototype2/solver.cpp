#include "solver.hpp"
#include "glbinding/gl/enum.h"
#include "glbinding/gl/functions.h"
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


#define TIME_CODE_BLOCK_CUSTOM2(timer_struct, enable_gpu, index, ...) \
    if (boolean(enable_gpu)) \
    { \
        /* No need to mention the GPU-timer overhead on the cpu-side */ \
        /* but i assume its negligeble */ \
        \
        timer_struct.m_cputimerBuf[index].begin(); \
        Time::GPUTimer::begin(timer_struct.m_gputimerBuf[index]); \
        __VA_ARGS__; \
        Time::GPUTimer::end(timer_struct.m_gputimerBuf[index]); \
        timer_struct.m_cputimerBuf[index].end(); \
    } else { \
        TIME_NAMESPACE_TIME_CODE_BLOCK(timer_struct.m_cputimerBuf[index], __VA_ARGS__); \
    } \


namespace proto2 {


static constexpr f32 MATERIAL_TYPE_FLUID = 2.0f;
static constexpr f32 MATERIAL_TYPE_SOLID = 1.0f;
static constexpr f32 MATERIAL_TYPE_EMPTY = 0.0f;


void FluidSolver::create()
{
    constexpr u32 particleCountX = 100;
    constexpr u32 particleCountY = 1;
    constexpr u32 particleCountZ = 100;
    m_gridSize        = { 128, 128, 128 };
    m_reductionFactor = 256;
    m_unitGridSize  = 1.0f;
    m_particleCount = { particleCountX, particleCountY, particleCountZ };
    m_dt            = 0.08f;
    // m_dt = 0.001f;
    m_density       = 1.0f;
    m_gravity       = -9.8f;
    

    for(auto& gpu_timer : m_timers.m_gputimerBuf) {
        gpu_timer.create();
    }
    m_pressureSolver.init(m_gridSize, m_unitGridSize, m_reductionFactor);
    markfmt("\
FluidSolver::create() =>\n\
    m_gridSize        (%4u, %4u, %4u)\n\
    m_reductionFactor %4u\n\
    m_particleCount   (%4u, %4u, %4u)\n\
    m_unitGridSize %f\n\
    m_dt           %f\n\
    m_Density      %f\n\
    m_gravity      %f\n\
",
    m_gridSize.x, m_gridSize.y, m_gridSize.z,
    m_reductionFactor,
    m_particleCount.x, m_particleCount.y, m_particleCount.z,
    m_unitGridSize,
    m_dt,
    m_density,
    m_gravity
);
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
    TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 0, advect(
        mr_prevTexU,
        mr_prevTexV,
        mr_prevTexW,
        mr_prevPos,
        mr_nextPos
    ));
    // TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 1, resetCellTypes());
    // TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 2, particleToGrid(
    //     mr_nextPos,
    //     mr_prevVel,
    //     mkr_cellTypeTexture,
    //     mr_prevTexU,
    //     mr_prevTexV,
    //     mr_prevTexW,
    //     mkr_uTexture2,
    //     mkr_vTexture2,
    //     mkr_wTexture2
    // ));
    // TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 4, normalize(
    //     mkr_uTexture2,
    //     mkr_vTexture2,
    //     mkr_wTexture2,
    //     mkr_uTexture3,
    //     mkr_vTexture3,
    //     mkr_wTexture3
    // ));
    // TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 5, boundaries(
    //     mkr_uTexture3,
    //     mkr_vTexture3,
    //     mkr_wTexture3,
    //     mkr_uTexture4,
    //     mkr_vTexture4,
    //     mkr_wTexture4
    // ));
    // TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 3, applyGravity(
    //     mkr_vTexture4,
    //     mkr_vTexture5
    // ));
    // TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 6, pressureSolve(
    //     mkr_uTexture4,
    //     mkr_vTexture5,
    //     mkr_wTexture4,
    //     mkr_cellTypeTexture,
    //     mkr_uTexture6,
    //     mkr_vTexture6,
    //     mkr_wTexture6
    // ));
    // TIME_CODE_BLOCK_CUSTOM2(m_timers, true, 7, gridToParticles(
    //     mkr_uTexture0,
    //     mkr_vTexture0,
    //     mkr_wTexture0,
    //     mkr_uTexture6,
    //     mkr_vTexture6,
    //     mkr_wTexture6,
    //     mr_nextPos,
    //     mr_prevVel,
    //     mr_nextVel
    // )); /* Verify args are correct */
    std::swap(mr_prevTexU, mr_nextTexU);
    std::swap(mr_prevTexV, mr_nextTexV);
    std::swap(mr_prevTexW, mr_nextTexW);
    std::swap(mr_prevPos , mr_nextPos);
    return;
}



void FluidSolver::initShaders()
{
    const std::array<ShaderData, mk_shaderCount> computeFiles = {{
        { "projects/program/source/prototype2/shaders/solver/advect.comp",        __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/exforces.comp",      __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/copy.comp",          __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/togrid.comp",        __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/boundarypass.comp",  __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/updatevel.comp",     __scast(u32, gl::GL_COMPUTE_SHADER)},
        { "projects/program/source/prototype2/shaders/solver/flip.comp",          __scast(u32, gl::GL_COMPUTE_SHADER)}
    }};
    const auto local_size = util::math::vec3u{ 32, 2, 1 };


    for(u32 i = 0; i < m_computePrograms.size(); ++i) {
        m_computePrograms[i].load(computeFiles[i]);
        m_computePrograms[i].resizeLocal(local_size.begin());
        m_computePrograms[i].resizeDispatchWithProblemSize(m_gridSize.begin(), 1);
    }
    for(u32 i = 0; i < m_computePrograms.size(); ++i) {
        if(!m_computePrograms[i].compile()) {
            ifcrash(true);
        } else
            m_computePrograms[i].info();
    }
    return;
}


void FluidSolver::initTextures()
{
    const std::array<TextureDescriptorArray::TextureFormat, mk_textureCount> texFormats = {
        /* U Component */
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        /* V Component */
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT },
        /* W Component */
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RG32F,   gl::GL_RG,   gl::GL_FLOAT },
        /* cell-types */
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F,    gl::GL_RED,  gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_R32F,    gl::GL_RED,  gl::GL_FLOAT }, 

        /* colour */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 
        /* velocity */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 
        /* position */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }, 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT },
        /* debug grid */ 
        TextureDescriptorArray::TextureFormat{ gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT }
    };
    const std::array<util::math::vec3u, mk_textureCount> texSizes = {
        util::math::vec3u{ m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },

        util::math::vec3u{ m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },

        util::math::vec3u{ m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        util::math::vec3u{ m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },

        m_gridSize,
        m_gridSize,

        m_particleCount,
        m_particleCount,
        m_particleCount,
        m_particleCount,
        m_particleCount,

        m_gridSize
    };


    m_texRegisters.create(mk_textureCount);
    for(u8 i = 0; i < texFormats.size(); ++i) {
        m_texRegisters[i].initialize(texFormats[i], texSizes[i]);
        m_texID[i] = i;
    }
    mr_prevTexU = mkr_uTexture0;
    mr_prevTexV = mkr_vTexture0;
    mr_prevTexW = mkr_wTexture0;
    mr_prevPos  = mkr_positionTexture0;
    mr_prevVel  = mkr_velocityTexture0;

    mr_nextTexU = mkr_uTexture1;
    mr_nextTexV = mkr_vTexture1;
    mr_nextTexW = mkr_wTexture1;
    mr_nextPos  = mkr_positionTexture1;
    mr_nextVel  = mkr_velocityTexture1;


    std::vector<util::math::vec4f> particlePositionBuf, colourBuf, gridPosBuf;
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


    particlePositionBuf.reserve(m_particleCount.x * m_particleCount.y * m_particleCount.z);
    for(i = 0; i < m_particleCount.x; ++i) {
        for(j = 0; j < m_particleCount.y; ++j) {
            for(k = 0; k < m_particleCount.z; ++k) {
                particlePositionBuf.push_back(util::math::vec4f{
                    __scast(f32, 2*i),
                    __scast(f32, 2*j),
                    __scast(f32, 2*k),
                    1.0f
                });
            }
        }
    }



    // m_cellTypeCPUside.resize(m_gridSize.x * m_gridSize.y * m_gridSize.z);
    // util::math::Tensor3View<f32> celltypes{ 
    //     m_cellTypeCPUside.data(), 
    //     __scast(i32, m_gridSize.x),
    //     __scast(i32, m_gridSize.y),
    //     __scast(i32, m_gridSize.z)
    // };
    // f32 material_type = MATERIAL_TYPE_EMPTY;
    // for(i = 0; i < m_gridSize.x; ++i) {
    //     for(j = 0; j < m_gridSize.y; ++j) {
    //         for(k = 0; k < m_gridSize.z; ++k) 
    //         {
    //             if( 
    //                 (i == 0 || i == m_gridSize.x - 1)
    //                 ||
    //                 (j == 0 || j == m_gridSize.y - 1)
    //                 ||
    //                 (k == 0 || k == m_gridSize.z - 1)
    //             ) {
    //                 material_type = MATERIAL_TYPE_SOLID;
    //             } else {
    //                 material_type = MATERIAL_TYPE_EMPTY;
    //             }
    //             celltypes(i, j, k) = material_type;
    //         }
    //     }
    // }
    m_cellTypeCPUside.reserve(m_gridSize.x * m_gridSize.y * m_gridSize.z);
    gridPosBuf.reserve(m_gridSize.x * m_gridSize.y * m_gridSize.z);
    for(i = 0; i < m_gridSize.x; ++i) {
        for(j = 0; j < m_gridSize.y; ++j) {
            for(k = 0; k < m_gridSize.z; ++k) 
            {
                bool isBnd = ( 
                    (i == 0 || i == m_gridSize.x - 1)
                    ||
                    (j == 0 || j == m_gridSize.y - 1)
                    ||
                    (k == 0 || k == m_gridSize.z - 1)
                );
                gridPosBuf.push_back(util::math::vec4f{ 2*i, 2*j, 2*k, 1u });
                m_cellTypeCPUside.push_back(isBnd ? MATERIAL_TYPE_SOLID : MATERIAL_TYPE_EMPTY);
            }
        }
    }



    colourBuf.resize(m_particleCount.x * m_particleCount.y * m_particleCount.z);
    std::generate_n(colourBuf.begin(), colourBuf.size(), get_randvec4);


    m_texRegisters[mkr_cellTypeTexture].set(m_cellTypeCPUside.data());
    m_texRegisters[mkr_initialCellTypeTexture].set(m_cellTypeCPUside.data());
    m_texRegisters[mkr_colourTexture].set(colourBuf.data());
    m_texRegisters[mkr_positionTexture0].set(particlePositionBuf.data());
    m_texRegisters[mkr_debugGridTexture].set(gridPosBuf.data());
    return;
}


void FluidSolver::waitVisibillity() {
    gl::glMemoryBarrier(gl::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | gl::GL_TEXTURE_FETCH_BARRIER_BIT);
    return;
}




void FluidSolver::advect(
    u32 velu,
    u32 velv,
    u32 velw,
    u32 readposition,
    u32 writeposition
) {
    mr_advect.bind();
    mr_advect.uniform1i("velu", 0);
    mr_advect.uniform1i("velv", 1);
    mr_advect.uniform1i("velw", 2);
    mr_advect.uniform1i("prevposition", 3);
    mr_advect.uniform1i("nextposition", 4);
    mr_advect.uniform1f("ku_gravity", m_gravity);
    mr_advect.uniform1f("ku_dt",      m_dt);
    mr_advect.uniform1f("ku_dl",      m_unitGridSize);
    m_texRegisters[velu].bindUnit(0);
    m_texRegisters[velv].bindUnit(1);
    m_texRegisters[velw].bindUnit(2);
    m_texRegisters[readposition].bindImage(3, gl::GL_READ_ONLY);
    m_texRegisters[writeposition].bindImage(4, gl::GL_WRITE_ONLY);
    mr_advect.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::applyGravity(
    u32 readv,
    u32 writev
) {
    mr_gravity.bind();
    mr_gravity.uniform1i("prevstaggeredv", 0);
    mr_gravity.uniform1i("nextstaggeredv", 1);
    mr_gravity.uniform1f("ku_gravity", m_gravity);
    mr_gravity.uniform1f("ku_dt",      m_dt);
    m_texRegisters[readv].bindImage(0, gl::GL_READ_ONLY);
    m_texRegisters[writev].bindImage(1, gl::GL_WRITE_ONLY);
    mr_gravity.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::resetCellTypes()
{
    copy(mkr_cellTypeTexture, mkr_initialCellTypeTexture);
    m_texRegisters[mr_prevTexU].clear();
    m_texRegisters[mr_prevTexV].clear();
    m_texRegisters[mr_prevTexW].clear();
    return;
}


void FluidSolver::particleToGrid(
    u32 currPos,
    u32 currVel,
    u32 writeCellTypes,
    u32 readu,
    u32 readv,
    u32 readw,
    u32 writeu,
    u32 writev,
    u32 writew
) {
    util::math::vec3f leftmostpos{0.0f};

    mr_stagger.bind();
    mr_stagger.uniform1i("positions",      0);
    mr_stagger.uniform1i("velocities",     1);
    mr_stagger.uniform1i("nextcelltypes",  2);
    mr_stagger.uniform1i("prevstaggeredu", 3);
    mr_stagger.uniform1i("prevstaggeredv", 4);
    mr_stagger.uniform1i("prevstaggeredw", 5);
    mr_stagger.uniform1i("nextstaggeredu", 6);
    mr_stagger.uniform1i("nextstaggeredv", 7);
    mr_stagger.uniform1i("nextstaggeredw", 8);
    mr_stagger.uniform3fv("u_bottomLeftmostPosition", leftmostpos.begin());
    mr_stagger.uniform1f("ku_dl", m_unitGridSize);
    m_texRegisters[currPos].bindUnit(0);
    m_texRegisters[currVel].bindUnit(1);
    m_texRegisters[writeCellTypes].bindImage(2, gl::GL_WRITE_ONLY);
    m_texRegisters[readu].bindImage(3, gl::GL_READ_ONLY);
    m_texRegisters[readv].bindImage(4, gl::GL_READ_ONLY);
    m_texRegisters[readw].bindImage(5, gl::GL_READ_ONLY);
    m_texRegisters[writeu].bindImage(6, gl::GL_WRITE_ONLY);
    m_texRegisters[writev].bindImage(7, gl::GL_WRITE_ONLY);
    m_texRegisters[writew].bindImage(8, gl::GL_WRITE_ONLY);
    mr_stagger.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::normalize(
    u32 readu,
    u32 readv,
    u32 readw,
    u32 writeu,
    u32 writev,
    u32 writew
) {
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
    m_texRegisters[readu].bindUnit(0);
    m_texRegisters[readv].bindUnit(1);
    m_texRegisters[readw].bindUnit(2);
    m_texRegisters[writeu].bindImage(3, gl::GL_WRITE_ONLY);
    m_texRegisters[writev].bindImage(4, gl::GL_WRITE_ONLY);
    m_texRegisters[writew].bindImage(5, gl::GL_WRITE_ONLY);
    mr_normalizevel.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::boundaries(
    u32 readu,
    u32 readv,
    u32 readw,
    u32 writeu,
    u32 writev,
    u32 writew
) {
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
        readu,
        readv,
        readw
    };
    u32 next[3] = {
        writeu,
        writev,
        writew
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


void FluidSolver::pressureSolve(
    u32 readu,
    u32 readv,
    u32 readw,
    u32 celltypes_texid,
    u32 writeu,
    u32 writev,
    u32 writew
) { 
    const f32 density_div_dt = m_density / m_dt;
    u32 pressure_texid = m_pressureSolver.solve(
        m_texRegisters[readu].id(),
        m_texRegisters[readv].id(),
        m_texRegisters[readw].id(),
        density_div_dt,
        m_texRegisters[celltypes_texid].id()
    );


    /* COMPUTE REQUIRES IMPLEMENTATION */
    mr_updatevel.bind();
    mr_updatevel.uniform1i("celltypes", 0);
    mr_updatevel.uniform1i("pressure",  1);
    mr_updatevel.uniform1i("velu",      2);
    mr_updatevel.uniform1i("velv",      3);
    mr_updatevel.uniform1i("velw",      4);
    gl::glBindTextureUnit(0, pressure_texid);
    m_texRegisters[mkr_cellTypeTexture].bindUnit(1);
    m_texRegisters[writeu].bindImage(2, gl::GL_WRITE_ONLY);
    m_texRegisters[writev].bindImage(3, gl::GL_WRITE_ONLY);
    m_texRegisters[writew].bindImage(4, gl::GL_WRITE_ONLY);
    mr_updatevel.dispatch();
    waitVisibillity();
    return;
}


void FluidSolver::gridToParticles(
    u32 velOldU,
    u32 velOldV,
    u32 velOldW,
    u32 velNewU,
    u32 velNewV,
    u32 velNewW,
    u32 currPosition,
    u32 currVelocity,
    u32 nextVelocity
) {
    mr_flip.bind();
    mr_flip.uniform1i("velOldU", 0);
    mr_flip.uniform1i("velOldV", 1);
    mr_flip.uniform1i("velOldW", 2);
    mr_flip.uniform1i("velNewU", 3);
    mr_flip.uniform1i("velNewV", 4);
    mr_flip.uniform1i("velNewW", 5);
    mr_flip.uniform1i("position", 6);
    mr_flip.uniform1i("currVelocity", 7);
    mr_flip.uniform1i("nextVelocity", 8);
    mr_flip.uniform1f("ku_dt", m_dt);
    mr_flip.uniform1f("ku_dl", m_unitGridSize);
    mr_flip.uniform1f("ku_flipRatio", m_flipRatio); /* init m_flipRatio */

    m_texRegisters[velOldU].bindUnit(0);
    m_texRegisters[velOldV].bindUnit(1);
    m_texRegisters[velOldW].bindUnit(2);
    m_texRegisters[velNewU].bindUnit(3);
    m_texRegisters[velNewV].bindUnit(4);
    m_texRegisters[velNewW].bindUnit(5);
    m_texRegisters[currPosition].bindUnit(6);
    m_texRegisters[currVelocity].bindUnit(7); /* ? */
    m_texRegisters[nextVelocity].bindImage(8, gl::GL_WRITE_ONLY); /* ? */
    mr_flip.dispatch(); /* check proper init of mr_flip */
    waitVisibillity();


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