#pragma once

namespace lost
{
	static const char* _baseVSCode =
		"#version 460 core\n"
		"layout(location = 0) in vec3 vertPos;\n"
		"layout(location = 1) in vec2 vertTexCoord;\n"
		"layout(location = 2) in vec4 vertColor;\n"
		"layout(location = 3) in vec3 vertNormal;\n"
		"layout(location = 4) in mat4 mvp;\n"
		"layout(location = 8) in mat4 model;\n"
		"out vec3 fragPos;\n"
		"out vec2 fragTexCoord;\n"
		"out vec4 fragColor;\n"
		"out vec3 fragNormal;\n"
		"out vec3 fragWorldNormal;"
		"void main() {\n"
		"	gl_Position = mvp * vec4(vertPos, 1.0);\n"
		"	fragPos = vertPos;\n"
		"	fragColor = vertColor;\n"
		"	fragTexCoord = vertTexCoord;\n"
		"	fragNormal = vertNormal;\n"
		"	fragWorldNormal = normalize(mat3(model) * vertNormal);\n"
		"}";

	static const char* _baseFSCode =
		"#version 460 core\n"
		"in vec3 fragPos;\n"
		"in vec2 fragTexCoord;\n"
		"in vec4 fragColor;\n"
		"in vec3 fragNormal;\n"
		"in vec3 fragWorldNormal;\n"
		"layout(location = 0) out vec4 finalColor;\n"
		"uniform sampler2D color;\n"
		"void main() {\n"
		"	finalColor = texture(color, fragTexCoord) * fragColor;\n"
		"}";

	static const char* _baseTextFSCode =
		"#version 460 core\n"
		"in vec3 fragPos;\n"
		"in vec2 fragTexCoord;\n"
		"in vec4 fragColor;\n"
		"in vec3 fragNormal;\n"
		"in vec3 fragWorldNormal;\n"
		"layout(location = 0) out vec4 finalColor;\n"
		"uniform sampler2D color;\n"
		"void main() {\n"
		"	float val = texture(color, fragTexCoord).r;\n"
		"	finalColor = vec4(fragColor.r, fragColor.g, fragColor.b, val);\n"
		"}";
}