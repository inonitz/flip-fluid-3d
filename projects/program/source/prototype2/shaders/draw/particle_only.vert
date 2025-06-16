#version 460 core
layout(binding = 0) uniform sampler3D positionTex;
layout(binding = 1) uniform sampler3D colourTex;


out vec4 fragColor;


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
    ivec3 texelPos = indexToTexelCoord3D(textureSize(positionTex, 0), gl_InstanceID);
    vec4 pos = texelFetch(positionTex, texelPos, 0);
    vec4 col = texelFetch(colourTex,   texelPos, 0);


    gl_Position = projection * view * model * vec4(pos.xyz, 1);
    fragColor   = col;
    return;
}  