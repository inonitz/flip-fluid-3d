#pragma once
#include <util/vec2.hpp>
#include <util/time.hpp>
#include "gl/gltimer.hpp"
#include "gl/texturearray.hpp"
#include "psolver.hpp"


namespace proto2 {

struct FluidSolverTimers 
{
    std::array<Time::GPUTimer,    10> m_gputimerBuf;
    std::array<Time::TimestampV2, 10> m_cputimerBuf;
    Time::GPUTimer&  mr_advectTimeGPU         = m_gputimerBuf[0];
    Time::GPUTimer&  mr_resetCellTypesTimeGPU = m_gputimerBuf[1];
    Time::GPUTimer&  mr_particleToGridTimeGPU = m_gputimerBuf[2];
    Time::GPUTimer&  mr_addExternalForcesGPU  = m_gputimerBuf[3];
    Time::GPUTimer&  mr_normalizeTimeGPU      = m_gputimerBuf[4];
    Time::GPUTimer&  mr_boundariesTimeGPU     = m_gputimerBuf[5];
    Time::GPUTimer&  mr_pressureSolveTimeGPU  = m_gputimerBuf[6];
    Time::GPUTimer&  mr_gridToParticleTimeGPU = m_gputimerBuf[7];

    Time::TimestampV2& mr_advectTimeCPU         = m_cputimerBuf[0];
    Time::TimestampV2& mr_resetCellTypesTimeCPU = m_cputimerBuf[1];
    Time::TimestampV2& mr_particleToGridTimeCPU = m_cputimerBuf[2];
    Time::TimestampV2& mr_addExternalForcesCPU  = m_cputimerBuf[3];
    Time::TimestampV2& mr_normalizeTimeCPU      = m_cputimerBuf[4];
    Time::TimestampV2& mr_boundariesTimeCPU     = m_cputimerBuf[5];
    Time::TimestampV2& mr_pressureSolveTimeCPU  = m_cputimerBuf[6];
    Time::TimestampV2& mr_gridToParticleTimeCPU = m_cputimerBuf[7];


    __force_inline u64 getCPUTime_ns(u8 option) {
        u64 total = 0;


        switch(option) 
        {
        case 0: /* Minimum */
            for(auto& timer : m_cputimerBuf) {
                total += timer.minimum();
            }
        break;
        case 1: /* Maximum */
            for(auto& timer : m_cputimerBuf) {
                total += timer.maximum();
            }
        break;
        case 2: /* Average */
            for(auto& timer : m_cputimerBuf) {
                total += timer.average();
            }
        break;
        case 3: /* Current-Sum */
            for(auto& timer : m_cputimerBuf) {
                total += timer.previousFrame();
            }
        break;
        }

    
        return total;
    }
    __force_inline u64 getGPUTime_ns() {
        u64 total = 0;
        for(auto& timer : m_gputimerBuf) {
            total += timer.previousFrame();
        }
        return total;
    }
};


struct FluidSolver
{
public:

    void create();
    void destroy();
    void iterateOnce();


    __force_inline TextureDescriptorArray::TextureAuxillary getPositionTexture() {
        return m_texRegisters[mr_prevPos];
        // return m_texRegisters[mkr_positionTexture0];
    }
    __force_inline TextureDescriptorArray::TextureAuxillary getColourTexture() {
        return m_texRegisters[mkr_colourTexture];
    }
    __force_inline TextureDescriptorArray::TextureAuxillary getCellTypeTexture() {
        return m_texRegisters[mkr_cellTypeTexture];
    }
    __force_inline TextureDescriptorArray::TextureAuxillary getDebugGridTexture() {
        return m_texRegisters[mkr_debugGridTexture];
    }
    FluidSolverTimers& getTimers() { 
        return m_timers;
    }
    __force_inline util::math::vec3u getGridSize() const {
        return m_gridSize;
    }
    __force_inline util::math::vec3u getParticleCount() const {
        return m_particleCount;
    }
    __force_inline u32 getParticleTotal() const {
        return m_particleCount.x * m_particleCount.y * m_particleCount.z;
    }
    __force_inline f32 getDt() const {
        return m_dt;
    }


private:

    void initShaders();
    void initTextures();

    void destroyShaders() {
        for(auto& prog : m_computePrograms) { prog.destroy(); }
        return;
    }
    void destroyTextures() {
        m_texRegisters.destroy();
        m_cellTypeCPUside.resize(0);
        return;
    }


    void waitVisibillity();


    void advect(
        u32 velu,
        u32 velv,
        u32 velw,
        u32 readposition,
        u32 writeposition
    );
    void applyGravity(
        u32 readv,
        u32 writev
    );
    void resetCellTypes();
    void particleToGrid(
        u32 currPos,
        u32 currVel,
        u32 writeCellTypes,
        u32 readu,
        u32 readv,
        u32 readw,
        u32 writeu,
        u32 writev,
        u32 writew
    );
    void normalize(
        u32 readu,
        u32 readv,
        u32 readw,
        u32 writeu,
        u32 writev,
        u32 writew
    );
    void boundaries(
        u32 readu,
        u32 readv,
        u32 readw,
        u32 writeu,
        u32 writev,
        u32 writew
    );
    void pressureSolve(
        u32 readu,
        u32 readv,
        u32 readw,
        u32 celltypes_texid,
        u32 writeu,
        u32 writev,
        u32 writew
    );
    void gridToParticles(
        u32 velOldU,
        u32 velOldV,
        u32 velOldW,
        u32 velNewU,
        u32 velNewV,
        u32 velNewW,
        u32 currPosition,
        u32 currVelocity,
        u32 nextVelocity
    );

    void copy(u32 dest, u32 src);

private:
    static constexpr u32 mk_textureCount = 29;
    static constexpr u32 mk_shaderCount  = 7;
    using ShaderBuffer = std::array<ComputeShader, mk_shaderCount>;
    using TextureIDBuf = std::array<u32, mk_textureCount>;


    FluidSolverTimers      m_timers;
    PressureSolver         m_pressureSolver;
    ShaderBuffer           m_computePrograms;
    std::vector<f32>       m_cellTypeCPUside;
    TextureDescriptorArray m_texRegisters;
    TextureIDBuf           m_texID;


    /* Simulator/Scene Config */
    util::math::vec3u m_gridSize;
    util::math::vec3u m_particleCount;
    u64 m_reductionFactor;
    f32 m_unitGridSize;
    f32 m_dt;
    f32 m_density;
    f32 m_gravity;
    f32 m_flipRatio;


    ComputeShader& mr_advect       = m_computePrograms[0];
    ComputeShader& mr_gravity      = m_computePrograms[1];
    ComputeShader& mr_copy         = m_computePrograms[2];
    ComputeShader& mr_stagger      = m_computePrograms[3];
    ComputeShader& mr_boundarypass = m_computePrograms[4];
    ComputeShader& mr_normalizevel = m_computePrograms[4];
    ComputeShader& mr_updatevel    = m_computePrograms[5];
    ComputeShader& mr_flip         = m_computePrograms[6];


    /* Particle Data */
    /* Staggered Grid Quantities - texture[i, j].x = some quantity, texture[i, j].y = weight */
    const u32& mkr_uTexture0 = m_texID[0x00];
    const u32& mkr_uTexture1 = m_texID[0x01];
    const u32& mkr_uTexture2 = m_texID[0x02];
    const u32& mkr_uTexture3 = m_texID[0x03];
    const u32& mkr_uTexture4 = m_texID[0x04];
    const u32& mkr_uTexture5 = m_texID[0x05];
    const u32& mkr_uTexture6 = m_texID[0x06];
    const u32& mkr_vTexture0 = m_texID[0x07];
    const u32& mkr_vTexture1 = m_texID[0x08];
    const u32& mkr_vTexture2 = m_texID[0x09];
    const u32& mkr_vTexture3 = m_texID[0x0A];
    const u32& mkr_vTexture4 = m_texID[0x0B];
    const u32& mkr_vTexture5 = m_texID[0x0C];
    const u32& mkr_vTexture6 = m_texID[0x0D];
    const u32& mkr_wTexture0 = m_texID[0x0E];
    const u32& mkr_wTexture1 = m_texID[0x0F];
    const u32& mkr_wTexture2 = m_texID[0x10];
    const u32& mkr_wTexture3 = m_texID[0x11];
    const u32& mkr_wTexture4 = m_texID[0x12];
    const u32& mkr_wTexture5 = m_texID[0x13];
    const u32& mkr_wTexture6 = m_texID[0x14];
    const u32& mkr_initialCellTypeTexture = m_texID[0x15];
    const u32& mkr_cellTypeTexture        = m_texID[0x16];
    const u32& mkr_colourTexture    = m_texID[0x17];
    const u32& mkr_velocityTexture0 = m_texID[0x18];
    const u32& mkr_velocityTexture1 = m_texID[0x19];
    const u32& mkr_positionTexture0 = m_texID[0x1a];
    const u32& mkr_positionTexture1 = m_texID[0x1b];
    const u32& mkr_debugGridTexture = m_texID[0x1c];

    /* Named References */
    u32 mr_prevTexU;
    u32 mr_prevTexV;
    u32 mr_prevTexW;
    u32 mr_prevPos;
    u32 mr_prevVel;

    u32 mr_nextTexU;
    u32 mr_nextTexV;
    u32 mr_nextTexW;
    u32 mr_nextPos;
    u32 mr_nextVel;
};


} /* namespace proto2 */