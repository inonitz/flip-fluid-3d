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
    Time::TimestampV2& mr_advectTimeCPU         = m_cputimerBuf[0];
    Time::TimestampV2& mr_resetCellTypesTimeCPU = m_cputimerBuf[1];
    Time::TimestampV2& mr_particleToGridTimeCPU = m_cputimerBuf[2];
    Time::TimestampV2& mr_addExternalForcesCPU  = m_cputimerBuf[3];
    Time::TimestampV2& mr_normalizeTimeCPU      = m_cputimerBuf[4];
    Time::TimestampV2& mr_boundariesTimeCPU     = m_cputimerBuf[5];
    Time::TimestampV2& mr_pressureSolveTimeCPU  = m_cputimerBuf[6];


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
    }
    __force_inline TextureDescriptorArray::TextureAuxillary getColourTexture() {
        return m_texRegisters[mkr_colourTexture];
    }
    FluidSolverTimers& getTimers() { 
        return m_timers;
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


    void advect();
    void resetCellTypes();
    void particleToGrid();
    void normalize();
    void boundaries();
    void pressureSolve();

    void copy(u32 dest, u32 src);

private:
    FluidSolverTimers            m_timers;
    PressureSolver               m_pressureSolver;
    std::array<ComputeShader, 5> m_computePrograms;
    TextureDescriptorArray       m_texRegisters;
    std::array<u32, 26>          m_texID;
    std::vector<f32>             m_cellTypeCPUside;


    /* Simulator/Scene Config */
    util::math::vec3u m_gridSize;
    util::math::vec3u m_particleCount;
    u64 m_reductionFactor;
    f32 m_unitGridSize;
    f32 m_dt;
    f32 m_gravity;


    ComputeShader& mr_advect       = m_computePrograms[0];
    ComputeShader& mr_copy         = m_computePrograms[1];
    ComputeShader& mr_stagger      = m_computePrograms[2];
    ComputeShader& mr_boundarypass = m_computePrograms[3];
    ComputeShader& mr_normalizevel = m_computePrograms[3];
    ComputeShader& mr_flip         = m_computePrograms[4];


    /* Particle Data */
    /* Staggered Grid Quantities - texture[i, j].x = some quantity, texture[i, j].y = weight */
    const u32& mkr_initialCellTypeTexture = m_texID[0x00];
    const u32& mkr_cellTypeTexture  = m_texID[0x01];
    const u32& mkr_velocityTexture  = m_texID[0x02];
    const u32& mkr_colourTexture    = m_texID[0x03];
    const u32& mkr_positionTexture0 = m_texID[0x04];
    const u32& mkr_positionTexture1 = m_texID[0x05];
    const u32& mkr_uTexture0        = m_texID[0x06];
    const u32& mkr_uTexture1        = m_texID[0x07];
    const u32& mkr_uTexture2        = m_texID[0x08];
    const u32& mkr_uTexture3        = m_texID[0x09];
    const u32& mkr_uTexture4        = m_texID[0x0a];
    const u32& mkr_vTexture0        = m_texID[0x0b];
    const u32& mkr_vTexture1        = m_texID[0x0c];
    const u32& mkr_vTexture2        = m_texID[0x0d];
    const u32& mkr_vTexture3        = m_texID[0x0e];
    const u32& mkr_vTexture4        = m_texID[0x0f];
    const u32& mkr_wTexture0        = m_texID[0x10];
    const u32& mkr_wTexture1        = m_texID[0x11];
    const u32& mkr_wTexture2        = m_texID[0x12];
    const u32& mkr_wTexture3        = m_texID[0x13];
    const u32& mkr_wTexture4        = m_texID[0x14];
    const u32& mkr_pTexture0        = m_texID[0x15];
    const u32& mkr_pTexture1        = m_texID[0x16];
    const u32& mkr_pTexture2        = m_texID[0x17];
    const u32& mkr_pTexture3        = m_texID[0x18];
    const u32& mkr_pTexture4        = m_texID[0x19];


    /* Named References */
    u32 mr_prevTexU;
    u32 mr_prevTexV;
    u32 mr_prevTexW;
    u32 mr_prevTexP;
    u32 mr_prevPos;

    u32 mr_nextTexU;
    u32 mr_nextTexV;
    u32 mr_nextTexW;
    u32 mr_nextTexP;
    u32 mr_nextPos;
};


} /* namespace proto2 */