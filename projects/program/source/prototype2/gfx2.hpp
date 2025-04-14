#pragma once
#include <util/base_type.h>
#include <util/time.hpp>
#include <util/vec2.hpp>
#include <glbinding/gl/enum.h>
#include "gl/shader2.hpp"
#include "gl/camera.hpp"


namespace proto2 {


struct GraphicsContext {
    std::array<ShaderProgramV2, 5> m_shaderPrograms;

    ShaderProgramV2& mr_drawVerticesOnly = m_shaderPrograms[0];
    ShaderProgramV2& mr_drawParticles    = m_shaderPrograms[1];
    ShaderProgramV2& mr_transferToGrid   = m_shaderPrograms[2];
    ShaderProgramV2& mr_applyBoundaryCnd = m_shaderPrograms[3];
    ShaderProgramV2& mr_advect           = m_shaderPrograms[4];

    u32 m_vaos[3];
    u32 m_buffers[2];
    u32 AttributeTextures[8];
    const u32& m_emptyVao  = m_vaos[0];
    const u32& m_cubeVao   = m_vaos[1];
    const u32& m_sphereVao = m_vaos[2];
    const u32& m_cubeVbo   = m_buffers[0];
    const u32& m_sphereVbo = m_buffers[1];
    const u32& m_positionTexture = AttributeTextures[0]; /* Particle Data */
    const u32& m_velocityTexture = AttributeTextures[1]; /* Particle Data */
    const u32& m_colourTexture   = AttributeTextures[2]; /* Particle Data */
    const u32& m_uTexture        = AttributeTextures[3]; /* Staggered Grid Quantities - texture[i, j].x = some quantity, texture[i, j].y = weight */
    const u32& m_vTexture        = AttributeTextures[4]; /* Staggered Grid Quantities - texture[i, j].x = some quantity, texture[i, j].y = weight */
    const u32& m_wTexture        = AttributeTextures[5]; /* Staggered Grid Quantities - texture[i, j].x = some quantity, texture[i, j].y = weight */
    const u32& m_pTexture        = AttributeTextures[6]; /* Staggered Grid Quantities - texture[i, j].x = some quantity, texture[i, j].y = weight */
    const u32& m_cellTypeTexture = AttributeTextures[7]; /* 3D Array for Cell types */

    const std::array<std::array<gl::GLenum, 3>, __carraysize(AttributeTextures)> m_textureFormats = {{
        { gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT },
        { gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT },
        { gl::GL_RGBA32F, gl::GL_RGBA, gl::GL_FLOAT },
        { gl::GL_RG32F, gl::GL_RG, gl::GL_FLOAT },
        { gl::GL_RG32F, gl::GL_RG, gl::GL_FLOAT },
        { gl::GL_RG32F, gl::GL_RG, gl::GL_FLOAT },
        { gl::GL_RG32F, gl::GL_RG, gl::GL_FLOAT },
        { gl::GL_R32F,  gl::GL_R,  gl::GL_FLOAT },
    }};
    std::array<util::math::vec3u, __carraysize(AttributeTextures)> km_textureSizes;


    util::math::vec3u m_gridSize;
    f32 m_unitGridSize;
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


} /* namespace proto1 */
