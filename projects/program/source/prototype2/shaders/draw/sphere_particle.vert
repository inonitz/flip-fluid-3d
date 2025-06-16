#version 460 core
layout(location = 0) in vec3 vertexPosition;
layout(binding = 0) uniform sampler3D positionTex;


#define MATERIAL_TYPE_FLUID (2.0f)
#define MATERIAL_TYPE_SOLID (1.0f)
#define MATERIAL_TYPE_EMPTY (0.0f)


out vec4 fragColor;


uniform uvec3 ku_particleCount;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


ivec2 indexToTexelCoord(in ivec2 size, int index) {
    int i = index / size.x;
    int j = index - i * size.x;
    return ivec2(i, j);
}

ivec3 indexToTexelCoord3D(in ivec3 size, int index) {
    int zDirection = index % size.z;
    int yDirection = (index / size.z) % size.y;
    int xDirection = index / (size.y * size.z);
    return ivec3(xDirection, yDirection, zDirection);
}


void main()
{
    ivec3 texelPos = indexToTexelCoord3D(ivec3(ku_particleCount), gl_InstanceID);
    vec4 pos = texelFetch(positionTex, texelPos, 0);


    gl_Position = projection * view * model * vec4(vertexPosition + pos.xyz, 1);
    fragColor   = vec4(0, 0, 1, 1);
    return;
}  