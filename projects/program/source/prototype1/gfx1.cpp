#include "gfx1.hpp"
#include "gl/shader2.hpp"
#include "glbinding/gl/enum.h"
#include <algorithm>
#include <util/random.hpp>
#include <glbinding/gl/gl.h>
#include <imgui/imgui.h>
#include <awc2/C/awc2.h>
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


namespace proto1 {


static std::vector<f32> verts_cube =
{
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
};


static std::vector<glm::vec3> verts_sphere;




void GraphicsContext::initialize()
{
    constexpr const char* shaderFiles[6] = {
        "projects/program/source/prototype1/shaders/position_only.vert",
        "projects/program/source/prototype1/shaders/position_only.frag",
        "projects/program/source/prototype1/shaders/shader.vert",
        "projects/program/source/prototype1/shaders/shader.frag",
        "projects/program/source/prototype1/shaders/stagger.comp",
        "projects/program/source/prototype1/shaders/advect.comp"
    };
    const std::array<ShaderData, 6> shaderMetadata = {{
        { shaderFiles[0], __scast(u32, gl::GL_VERTEX_SHADER  )},
        { shaderFiles[1], __scast(u32, gl::GL_FRAGMENT_SHADER)},
        { shaderFiles[2], __scast(u32, gl::GL_VERTEX_SHADER  )},
        { shaderFiles[3], __scast(u32, gl::GL_FRAGMENT_SHADER)},
        { shaderFiles[4], __scast(u32, gl::GL_COMPUTE_SHADER )},
        { shaderFiles[5], __scast(u32, gl::GL_COMPUTE_SHADER )}
    }};
    constexpr u32 particleCountX = 100;
    constexpr u32 particleCountY = 100;
    constexpr u32 particleCountZ = 100;
    const util::math::vec3u particleCount{ particleCountX, particleCountY, particleCountZ };


    gl::glEnable(gl::GL_DEPTH_TEST);
    gl::glEnable(gl::GL_BLEND);
    gl::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl::glPointSize(10);


    generateSphere(24, 24, 1, verts_sphere);

    mr_drawVerticesOnly.createFrom({ shaderMetadata[0], shaderMetadata[1] });
    mr_drawParticles.createFrom   ({ shaderMetadata[2], shaderMetadata[3] });
    mr_transferToGrid.createFrom  ({ shaderMetadata[4] });
    mr_advect.createFrom          ({ shaderMetadata[5] });
    for(u32 i = 0; i < m_shaderPrograms.size(); ++i) {
        if(!m_shaderPrograms[i].compile())
            ifcrash(true);
    }
    

    gl::glCreateVertexArrays(__carraysize(m_vaos), m_vaos);
    gl::glCreateBuffers(__carraysize(m_buffers), m_buffers);
    gl::glCreateTextures(gl::GL_TEXTURE_3D, __carraysize(AttributeTextures), AttributeTextures);


    gl::glNamedBufferStorage(m_cubeVbo, verts_cube.size() * sizeof(f32), verts_cube.data(), gl::GL_DYNAMIC_STORAGE_BIT);
    gl::glVertexArrayVertexBuffer(m_cubeVao, 0, m_cubeVbo, 0, sizeof(f32) * 3);
    gl::glEnableVertexArrayAttrib(m_cubeVao, 0);
    gl::glVertexArrayAttribFormat(m_cubeVao, 0, 3, gl::GL_FLOAT, gl::GL_FALSE, 0);
    gl::glVertexArrayAttribBinding(m_cubeVao, 0, 0);

    gl::glNamedBufferStorage(m_sphereVbo, verts_sphere.size() * sizeof(glm::vec3), verts_sphere.data(), gl::GL_DYNAMIC_STORAGE_BIT);
    gl::glVertexArrayVertexBuffer(m_sphereVao, 0, m_sphereVbo, 0, sizeof(glm::vec3));
    gl::glEnableVertexArrayAttrib(m_sphereVao, 0);
    gl::glVertexArrayAttribFormat(m_sphereVao, 0, 3, gl::GL_FLOAT, gl::GL_FALSE, 0);
    gl::glVertexArrayAttribBinding(m_sphereVao, 0, 0);


    // m_gridSize      = { 128, 128, 128 };
    m_gridSize      = { 512, 512, 512 };
    m_unitGridSize  = 1.0f;
    m_particleCount = particleCountX * particleCountY * particleCountZ;
    m_dt            = 0.08f;
    km_textureSizes = {
        particleCount,
        particleCount,
        particleCount,
        { m_gridSize.x + 1, m_gridSize.y, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y + 1, m_gridSize.z },
        { m_gridSize.x, m_gridSize.y, m_gridSize.z + 1 },
        m_gridSize,
        m_gridSize
    };
    for(u32 i = 0; i < __carraysize(AttributeTextures); ++i) {
        const auto& tex      = AttributeTextures[i];
        const auto& gridSize = km_textureSizes[i];
        const auto& texFmt   = m_textureFormats[i];
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_WRAP_R, gl::GL_CLAMP_TO_EDGE);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        gl::glTextureParameteri(tex, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        gl::glTextureStorage3D(tex, 1, texFmt[0], gridSize.x, gridSize.y, gridSize.z);
        gl::glClearTexImage(tex, 0, texFmt[1], texFmt[2], nullptr);
    }


    std::vector<util::math::vec4f> positionBuf, colourBuf;
    auto get_randvec4 = []() -> util::math::vec4f {
        return util::math::vec4f{ random32f(), random32f(), random32f(), 0.0f };
    };


    for(u32 i = 0; i < particleCountX; ++i) {
        for(u32 j = 0; j < particleCountY; ++j) {
            for(u32 k = 0; k < particleCountZ; ++k) {
                positionBuf.push_back(util::math::vec4f{
                    __scast(f32, i) / particleCountX,
                    __scast(f32, j) / particleCountY,
                    __scast(f32, k) / particleCountZ,
                    1.0f
                });
            }
        }
    }


    colourBuf.resize(m_particleCount);
    std::generate_n(colourBuf.begin(), colourBuf.size(), get_randvec4);


    gl::glTextureSubImage3D(m_positionTexture, 0, 
        0, 0, 0, 
        particleCount.x, particleCount.y, particleCount.z,
        m_textureFormats[0][1],
        m_textureFormats[0][2],
        positionBuf.data()
    );
    gl::glTextureSubImage3D(m_colourTexture, 0, 
        0, 0, 0, 
        particleCount.x, particleCount.y, particleCount.z, 
        m_textureFormats[0][1],
        m_textureFormats[0][2],
        colourBuf.data()
    );



    m_camera = new CameraFPS();
    return;
}


void GraphicsContext::destroy()
{
    gl::glDeleteVertexArrays(__carraysize(m_vaos), m_vaos);
    gl::glDeleteBuffers(__carraysize(m_buffers), m_buffers);
    gl::glDeleteTextures(__carraysize(AttributeTextures), AttributeTextures);

    mr_drawVerticesOnly.destroy();
    mr_drawParticles.destroy();
    mr_transferToGrid.destroy();
    mr_advect.destroy();
    return;
}


void GraphicsContext::update()
{
    renderimgui();
    render();
    return;
}


void GraphicsContext::renderimgui()
{    
    ImGui::Begin("Diagnostics Window");
    ImGui::Text("%llu [ns] Frame Time", awc2getCurrentContextFrameTime());
    ImGui::Text("%u FPS", __scast(u32, 1e+9 / awc2getCurrentContextFrameTime()));

    ImGui::SeparatorText("Camera");
    ImGui::Text("Position  %s\n", glm::to_string(m_camera->getPosition()).c_str());
    ImGui::Text("Direction %s\n", glm::to_string(m_camera->getDirection()).c_str());


    ImGui::SeparatorText("Projection Matrix");
    bool testany = false;
    testany = testany || ImGui::DragFloat("Field Of View", &m_projectionParams[0], 0.1f, 0.0f, 90.0f );
    testany = testany || ImGui::DragFloat("Near Plane",    &m_projectionParams[1], 0.1f, 0.0f, 10.0f );
    testany = testany || ImGui::DragFloat("Far  Plane",    &m_projectionParams[2], 0.1f, 0.0f, 100.0f);
    if(testany) {
        m_camera->updateProjectionParameters(m_projectionParams[0], m_projectionParams[1], m_projectionParams[2]);
    }

    testany = false;
    ImGui::SeparatorText("Model Matrices");
    testany = testany || ImGui::DragFloat3("Particles", glm::value_ptr(m_particleTranslate), 0.01f, -100.0f, 100.0f);
    testany = testany || ImGui::DragFloat3("Cube     ", glm::value_ptr(m_cubeTranslate),     0.01f, -100.0f, 100.0f);
    testany = testany || ImGui::DragFloat3("Sphere   ", glm::value_ptr(m_sphereTranslate),   0.01f, -100.0f, 100.0f);


    if(awc2isMouseButtonPressed(AWC2_MOUSEBUTTON_RIGHT)) {
        awc2setCursorMode(AWC2_CURSORMODE_DISABLED);
        m_camera->update(m_dt);
    }
    if(awc2isMouseButtonReleased(AWC2_MOUSEBUTTON_RIGHT)) {
        awc2setCursorMode(AWC2_CURSORMODE_NORMAL);
    }


    ImGui::End();
}


void GraphicsContext::render()
{
    glm::mat4x4 model = glm::identity<glm::mat4x4>();


    gl::glBindVertexArray(m_emptyVao);
    gl::glBindTextureUnit(0, m_positionTexture);
    gl::glBindTextureUnit(1, m_colourTexture);
    model = glm::translate(glm::identity<glm::mat4x4>(), -m_particleTranslate);
    mr_drawParticles.bind();
    mr_drawParticles.uniform1i("positionTex", 0);
    mr_drawParticles.uniform1i("colourTex",   1);
    mr_drawParticles.uniformMatrix4fv("projection", glm::value_ptr(m_camera->getProjection()) );
    mr_drawParticles.uniformMatrix4fv("view",       glm::value_ptr(m_camera->getView())       );
    mr_drawParticles.uniformMatrix4fv("model",      glm::value_ptr(model));
    gl::glDrawArraysInstanced(gl::GL_POINTS, 0, 1, m_particleCount);
    

    gl::glBindVertexArray(m_cubeVao);
    model = glm::translate(glm::identity<glm::mat4x4>(), -m_cubeTranslate);
    mr_drawVerticesOnly.bind();
    mr_drawVerticesOnly.uniformMatrix4fv("projection", glm::value_ptr(m_camera->getProjection()) );
    mr_drawVerticesOnly.uniformMatrix4fv("view",       glm::value_ptr(m_camera->getView())       );
    mr_drawVerticesOnly.uniformMatrix4fv("model",      glm::value_ptr(model));
    gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 36);


    gl::glBindVertexArray(m_sphereVao);
    model = glm::translate(glm::identity<glm::mat4x4>(), -m_sphereTranslate);
    mr_drawVerticesOnly.bind();
    mr_drawVerticesOnly.uniformMatrix4fv("projection", glm::value_ptr(m_camera->getProjection()) );
    mr_drawVerticesOnly.uniformMatrix4fv("view",       glm::value_ptr(m_camera->getView())       );
    mr_drawVerticesOnly.uniformMatrix4fv("model",      glm::value_ptr(model));
    gl::glDrawArrays(gl::GL_TRIANGLES, 0, verts_sphere.size());
    return;
}


void GraphicsContext::generateSphere(i32 numLatitudeLines, i32 numLongitudeLines, i32 radius, std::vector<glm::vec3>& outputVertices)
{
    /*
        Thanks to:
        https://gamedev.stackexchange.com/questions/150191/opengl-calculate-uv-sphere-vertices
        Also a pastebin if the link is dead:
        https://pastebin.com/276PwiYM
    */

    // One vertex at every latitude-longitude intersection,
    // plus one for the north pole and one for the south.
    // One meridian serves as a UV seam, so we double the vertices there.
    int numVertices = (numLatitudeLines * (numLongitudeLines + 1)) + 2;
    int numTriangles = numLatitudeLines * numLongitudeLines * 2;
    float latitudeSpacing = 1.0f / (numLatitudeLines + 1.0f);
    float longitudeSpacing = 1.0f / numLongitudeLines;
    int v = 1;

    std::vector<glm::vec3> positions(numVertices);
    std::vector<glm::vec2> texcoords(numVertices);
    std::vector<glm::vec3> vertices(numTriangles * 3);


    // North pole.
    positions[0] = glm::vec3(0, radius, 0);
    texcoords[0] = glm::vec2(0, 1);
    // South pole.
    positions[numVertices - 1] = glm::vec3(0, -radius, 0);
    texcoords[numVertices - 1] = glm::vec2(0, 0);


    for (int latitude = 0; latitude < numLatitudeLines; latitude++)
    {
        for (int longitude = 0; longitude <= numLongitudeLines; longitude++)
        {
            // Scale coordinates into the 0...1 texture coordinate range,
            // with north at the top (y = 1).
            texcoords[v] = glm::vec2(
                                longitude * longitudeSpacing,
                                1.0f - ((latitude + 1) * latitudeSpacing)
                            );
            // Convert to spherical coordinates:
            // theta is a longitude angle (around the equator) in radians.
            // phi is a latitude angle (north or south of the equator).
            float theta = (texcoords[v].x * 2.0f * glm::pi<f32>());
            float phi   = ((texcoords[v].y - 0.5f) * glm::pi<f32>());
            // This determines the radius of the ring of this line of latitude.
            // It's widest at the equator, and narrows as phi increases/decreases.
            float c = cosf(phi);
            // Usual formula for a vector in spherical coordinates.
            // You can exchange x & z to wind the opposite way around the sphere.
            positions[v] = glm::vec3(
                (c * cosf(theta)),
                sinf(phi),
                (c * sinf(theta))) * __scast(f32, radius);

            v++;
        }
    }


    // Convert Vertices to triangles
    v = 0;
    for (int i = 0; i < numLongitudeLines; i++)
    {
        vertices[v++] = glm::vec3(positions[0]);
        vertices[v++] = glm::vec3(positions[i + 2]);
        vertices[v++] = glm::vec3(positions[i + 1]);
    }
    // Each row has one more unique vertex than there are lines of longitude,
    // since we double a vertex at the texture seam.
    int rowLength = numLongitudeLines + 1;
    for (int latitude = 0; latitude < numLatitudeLines - 1; latitude++)
    {
        // Plus one for the pole.
        int rowStart = (latitude * rowLength) + 1;
        for (int longitude = 0; longitude < numLongitudeLines; longitude++)
        {
            int firstCorner = rowStart + longitude;
            // First triangle of quad: Top-Left, Bottom-Left, Bottom-Right
            vertices[v++] = glm::vec3(positions[firstCorner]);
            vertices[v++] = glm::vec3(positions[firstCorner + rowLength + 1]);
            vertices[v++] = glm::vec3(positions[firstCorner + rowLength]);
            // Second triangle of quad: Top-Left, Bottom-Right, Top-Right
            vertices[v++] = glm::vec3(positions[firstCorner]);
            vertices[v++] = glm::vec3(positions[firstCorner + 1]);
            vertices[v++] = glm::vec3(positions[firstCorner + rowLength + 1]);
        }
    }


    int pole = positions.size() - 1;
    int bottomRow = ((numLatitudeLines - 1) * rowLength) + 1;
    for (int i = 0; i < numLongitudeLines; i++)
    {
        vertices[v++] = glm::vec3(positions[pole]);
        vertices[v++] = glm::vec3(positions[bottomRow + i]);
        vertices[v++] = glm::vec3(positions[bottomRow + i + 1]);
    }


    outputVertices.swap(vertices);
    return;
}


} /* namespace proto1 */