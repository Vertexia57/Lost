#version 460 core

in vec3 fragPos;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragWorldNormal;

layout (location=0) out vec4 finalColor;

uniform sampler2D color;
uniform sampler2D specular;
uniform sampler2D normal;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 ambientLightColor;
uniform vec2 resolution;

float halfLambert(vec3 normal, vec3 lightDir)
{
    float NdotL = max(0.0, dot(normal, lightDir));
    return pow(NdotL * 0.5 + 0.5, 2.0);
}

void main() {
    vec3 lightDir = normalize(vec3(0.0f, 0.0f, 1.0f));
    float lightVal = halfLambert(fragWorldNormal, lightDir);
    vec4 textureColor = texture(color, fragTexCoord);

    finalColor = vec4(textureColor.rgb * lightVal, textureColor.a) * texture(specular, fragTexCoord) * texture(normal, fragTexCoord);
}