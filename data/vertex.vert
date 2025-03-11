#version 460 core

layout (location=0) in vec3 vertPos;
layout (location=1) in vec2 vertTexCoord;
layout (location=2) in vec4 vertColor;
layout (location=3) in vec3 vertNormal;

layout (location=4) in mat4 mvp;
layout (location=8) in mat4 model;
out vec3 fragPos;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragWorldNormal;

out mat4 fragModel;
out mat3 TBN;

void main() {
    gl_Position = mvp * vec4(vertPos, 1.0);

    fragPos = (model * vec4(vertPos, 1.0)).xyz;
    fragColor = vertColor;
    fragTexCoord = vertTexCoord;
    fragNormal = vertNormal;
	fragWorldNormal = normalize(mat3(model) * vertNormal);

    fragModel = model;
}