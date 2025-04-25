#include "gfx2.hpp"
#include "gl/shader2.hpp"
#include <util/marker2.hpp>
#include <glbinding/gl/gl.h>
#include <imgui/imgui.h>
#include <awc2/C/awc2.h>
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


namespace proto2 {


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


static std::vector<f32> celltype_resetbuf;




void GraphicsContext::initialize()
{
    gl::glEnable(gl::GL_DEPTH_TEST);
    gl::glEnable(gl::GL_BLEND);
    gl::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl::glPointSize(10);

    m_solver.create();
    initShaders();
    initBuffers();
    
    m_camera = new CameraFPS(
        glm::vec3{ -5.0f, 1.0f, 2.5f },
        glm::vec3(0, 1.0f, 0),
        0, 0, 2.5f, 0.1f, 100.0f,
        45.0f
    );
    return;
}


void GraphicsContext::destroy()
{
    delete m_camera;
    destroyBuffers();
    destroyShaders();
    m_solver.destroy();
    return;
}


void GraphicsContext::update()
{
    m_solver.iterateOnce();
    renderimgui();
    render();
    return;
}




void GraphicsContext::initShaders()
{
    const std::array<ShaderData, 4> graphicsFiles = {{
        { "projects/program/source/prototype2/shaders/position_only.vert", __scast(u32, gl::GL_VERTEX_SHADER  ) },
        { "projects/program/source/prototype2/shaders/position_only.frag", __scast(u32, gl::GL_FRAGMENT_SHADER) },
        { "projects/program/source/prototype2/shaders/shader.vert",        __scast(u32, gl::GL_VERTEX_SHADER  ) },
        { "projects/program/source/prototype2/shaders/shader.frag",        __scast(u32, gl::GL_FRAGMENT_SHADER) }
    }};


    for(u32 i = 0; i < graphicsFiles.size(); i += 2) 
    {
        m_shaderPrograms[i / 2].createFrom({ graphicsFiles[i], graphicsFiles[i + 1] });
        if(!m_shaderPrograms[i / 2].compile())
            ifcrash(true);
    }
    return;
}

void GraphicsContext::destroyShaders()
{
    for(u32 i = 0; i < m_shaderPrograms.size(); ++i) {
        m_shaderPrograms[i].destroy();
    }
    return;
}


void GraphicsContext::initBuffers()
{
    generateSphere(24, 24, 1, verts_sphere); /* Generate vertices for sphere buffer */


    gl::glCreateVertexArrays(__carraysize(m_vaos), m_vaos);
    gl::glCreateBuffers(__carraysize(m_buffers), m_buffers);
    gl::glNamedBufferStorage(mr_cubeVbo, verts_cube.size() * sizeof(f32), verts_cube.data(), gl::GL_DYNAMIC_STORAGE_BIT);
    gl::glVertexArrayVertexBuffer(mr_cubeVao, 0, mr_cubeVbo, 0, sizeof(f32) * 3);
    gl::glEnableVertexArrayAttrib(mr_cubeVao, 0);
    gl::glVertexArrayAttribFormat(mr_cubeVao, 0, 3, gl::GL_FLOAT, gl::GL_FALSE, 0);
    gl::glVertexArrayAttribBinding(mr_cubeVao, 0, 0);

    gl::glNamedBufferStorage(mr_sphereVbo, verts_sphere.size() * sizeof(glm::vec3), verts_sphere.data(), gl::GL_DYNAMIC_STORAGE_BIT);
    gl::glVertexArrayVertexBuffer(mr_sphereVao, 0, mr_sphereVbo, 0, sizeof(glm::vec3));
    gl::glEnableVertexArrayAttrib(mr_sphereVao, 0);
    gl::glVertexArrayAttribFormat(mr_sphereVao, 0, 3, gl::GL_FLOAT, gl::GL_FALSE, 0);
    gl::glVertexArrayAttribBinding(mr_sphereVao, 0, 0);
    return;
}

void GraphicsContext::destroyBuffers()
{
    gl::glDeleteVertexArrays(__carraysize(m_vaos), m_vaos);
    gl::glDeleteBuffers(__carraysize(m_buffers), m_buffers);
    verts_sphere.resize(0);
    verts_cube.resize(0);
    return;
}




void GraphicsContext::renderimgui()
{
    auto fluid_solver_time_measurements = m_solver.getTimers();
    auto f_timers = fluid_solver_time_measurements;
    static const char* imgui_diagnostics_text = "\
Compute Time\n\
CPU - computepass() \n\
- %-6.4f [total]\n\
- %-6.4f [advect]\n\
- %-6.4f [resetCellTypes]\n\
- %-6.4f [particleToGrid]\n\
- %-6.4f [normalize]\n\
- %-6.4f [boundaries]\n\
- %-6.4f [pressureSolve]\n\
GPU - computepass() \n\
- %-6.4f [total]\n\
- %-6.4f [advect]\n\
- %-6.4f [resetCellTypes]\n\
- %-6.4f [particleToGrid]\n\
- %-6.4f [normalize]\n\
- %-6.4f [boundaries]\n\
- %-6.4f [pressureSolve]\n\
";


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
        m_camera->update(m_solver.getDt());
    }
    if(awc2isMouseButtonReleased(AWC2_MOUSEBUTTON_RIGHT)) {
        awc2setCursorMode(AWC2_CURSORMODE_NORMAL);
    }


    ImGui::SeparatorText("Time measurements");
    ImGui::Text(imgui_diagnostics_text,
        f_timers.getCPUTime_ns(3) * 1e-6,
        f_timers.mr_advectTimeCPU.previousFrame() * 1e-6,
        f_timers.mr_resetCellTypesTimeCPU.previousFrame() * 1e-6,
        f_timers.mr_particleToGridTimeCPU.previousFrame() * 1e-6,
        f_timers.mr_normalizeTimeCPU.previousFrame() * 1e-6,
        f_timers.mr_boundariesTimeCPU.previousFrame() * 1e-6,
        f_timers.mr_pressureSolveTimeCPU.previousFrame() * 1e-6,
        f_timers.getGPUTime_ns() * 1e-6, 
        f_timers.mr_advectTimeGPU.previousFrame() * 1e-6,
        f_timers.mr_resetCellTypesTimeGPU.previousFrame() * 1e-6,
        f_timers.mr_particleToGridTimeGPU.previousFrame() * 1e-6,
        f_timers.mr_normalizeTimeGPU.previousFrame() * 1e-6,
        f_timers.mr_boundariesTimeGPU.previousFrame() * 1e-6,
        f_timers.mr_pressureSolveTimeGPU.previousFrame() * 1e-6
    );


    ImGui::End();
}


void GraphicsContext::render()
{
    glm::mat4x4 model = glm::identity<glm::mat4x4>();


    gl::glBindVertexArray(mr_emptyVao);
    m_solver.getPositionTexture().bindUnit(0);
    m_solver.getColourTexture().bindUnit(1);
    model = glm::translate(glm::identity<glm::mat4x4>(), -m_particleTranslate);
    mr_drawParticles.bind();
    mr_drawParticles.uniform1i("positionTex", 0);
    mr_drawParticles.uniform1i("colourTex",   1);
    mr_drawParticles.uniformMatrix4fv("projection", glm::value_ptr(m_camera->getProjection()) );
    mr_drawParticles.uniformMatrix4fv("view",       glm::value_ptr(m_camera->getView())       );
    mr_drawParticles.uniformMatrix4fv("model",      glm::value_ptr(model));
    gl::glDrawArraysInstanced(gl::GL_POINTS, 0, 1, m_solver.getParticleTotal());
    

    gl::glBindVertexArray(mr_cubeVao);
    model = glm::translate(glm::identity<glm::mat4x4>(), -m_cubeTranslate);
    mr_drawVerticesOnly.bind();
    mr_drawVerticesOnly.uniformMatrix4fv("projection", glm::value_ptr(m_camera->getProjection()) );
    mr_drawVerticesOnly.uniformMatrix4fv("view",       glm::value_ptr(m_camera->getView())       );
    mr_drawVerticesOnly.uniformMatrix4fv("model",      glm::value_ptr(model));
    gl::glDrawArrays(gl::GL_TRIANGLE_STRIP, 0, 36);


    gl::glBindVertexArray(mr_sphereVao);
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


} /* namespace proto2 */