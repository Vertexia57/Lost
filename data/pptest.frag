#version 460 core

in vec3 fragPos;
in vec2 fragTexCoord;
in vec4 fragColor;

layout (location=0) out vec4 finalColor;

uniform sampler2D color;

uniform vec2 resolution;

vec2 offset = 1.0f / resolution;

vec2 offsets[9] = vec2[]
(
    vec2(-offset.x, -offset.y), vec2(0.0f, -offset.y), vec2(offset.x, -offset.y),
    vec2(-offset.x, 0.0f)     , vec2(0.0f, 0.0f)     , vec2(offset.x, 0.0f),
    vec2(-offset.x, offset.y) , vec2(0.0f, offset.y) , vec2(offset.x, offset.y)  
);

float kernel[9] = float[]
(
    1,  1, 1,
    1, -8, 1,
    1,  1, 1
);

void main() {
    vec3 pColor = vec3(0.0f);
    for (int i = 0; i < 9; i++)
        pColor += vec3(texture(color, fragTexCoord + offsets[i]).rgb * kernel[i]);

    finalColor = vec4(pColor, 1.0f);
}