#pragma once
#include "gl/shader2.hpp"
#include "gl/camera.hpp"
#include "solver.hpp"


namespace proto2 {


struct GraphicsContext {
    FluidSolver                    m_solver;
    std::array<ShaderProgramV2, 2> m_shaderPrograms;
    u32 m_vaos[3];
    u32 m_buffers[2];

    ShaderProgramV2& mr_drawVerticesOnly = m_shaderPrograms[0];
    ShaderProgramV2& mr_drawParticles    = m_shaderPrograms[1];
    const u32& mr_emptyVao  = m_vaos[0];
    const u32& mr_cubeVao   = m_vaos[1];
    const u32& mr_sphereVao = m_vaos[2];
    const u32& mr_cubeVbo   = m_buffers[0];
    const u32& mr_sphereVbo = m_buffers[1];


    f32       m_projectionParams[3]{ 45.0f, 0.1f, 100.0f };
    glm::vec3 m_particleTranslate{ 0.0f, 0.0f, -1.5f };
    glm::vec3 m_cubeTranslate    { 0.0f, 0.0f,  2.2f };
    glm::vec3 m_sphereTranslate  { 0.0f, 0.0f,  0.0f };
    CameraFPS* m_camera;


    void initialize();
    void destroy();
    void update();
private:

    void initShaders();
    void destroyShaders();
    
    void initBuffers();
    void destroyBuffers();

    void renderimgui();
    void render();


    void generateSphere(
        i32 numLatitudeLines, 
        i32 numLongitudeLines, 
        i32 radius, 
        std::vector<glm::vec3>& outputVertices
    );
};


} /* namespace proto2 */
