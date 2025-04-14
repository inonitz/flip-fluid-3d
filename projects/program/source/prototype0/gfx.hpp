#pragma once
#include <util/base_type.h>
#include <util/time.hpp>
#include "gl/shader2.hpp"
#include "gl/camera.hpp"


struct GraphicsContext {
    ShaderProgramV2 m_draw;
    ShaderProgramV2 m_drawVerticesOnly;
    ShaderProgramV2 m_compute;
    

    u32 m_vaos[3];
    u32 m_buffers[2];
    u32 AttributeTextures[5];
    u32& m_emptyVao  = m_vaos[0];
    u32& m_cubeVao   = m_vaos[1];
    u32& m_sphereVao = m_vaos[2];
    u32& m_cubeVbo   = m_buffers[0];
    u32& m_sphereVbo = m_buffers[1];
    u32& m_positionTexture = AttributeTextures[0];
    u32& m_colourTexture   = AttributeTextures[1];

    u32 m_particleCount;
    f32 m_dt;


    f32       m_projectionParams[3]{ 45.0f, 0.1f, 100.0f };
    glm::vec3 m_particleTranslate{ 0.0f, 0.0f, -1.5f };
    glm::vec3 m_cubeTranslate    { 0.0f, 0.0f,  2.2f };
    glm::vec3 m_sphereTranslate  { 0.0f, 0.0f,  0.0f };
    CameraFPS* m_camera;


    void initialize();
    void destroy();
    void update();
private:
    void generateSphere(
        i32 numLatitudeLines, 
        i32 numLongitudeLines, 
        i32 radius, 
        std::vector<glm::vec3>& outputVertices
    );
    void renderimgui();
    void render();

};



