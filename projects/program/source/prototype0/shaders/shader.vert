#version 460 core
uniform sampler2D positionTex;
uniform sampler2D colourTex;


out vec4 fragColor;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


ivec2 indexToTexelCoord(in ivec2 size, int index) {
    int i = index / size.x;
    int j = index - i * size.x;
    return ivec2(i, j);
}


void main()
{
    ivec2 texelPos = indexToTexelCoord(textureSize(positionTex, 0), gl_InstanceID);
    vec4 pos = texelFetch(positionTex, texelPos, 0);
    vec4 col = texelFetch(colourTex,   texelPos, 0);

    // vec4 transformed_pos = projection * view * model * pos;
    // gl_Position =  vec4(pos.xy, 0, 1);
    gl_Position = projection * view * model * vec4(pos.xyz, 1);
    fragColor   = col;
    return;
}  