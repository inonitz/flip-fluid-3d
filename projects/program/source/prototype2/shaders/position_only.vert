#version 460 core
layout(location = 0) in vec3 vertexPosition;


out vec4 fragColor;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


void main()
{
    gl_Position = projection * view * model * vec4(vertexPosition, 1);
    fragColor   = vec4(1.0f);
    return;
}
