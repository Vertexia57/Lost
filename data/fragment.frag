#version 460 core

in vec3 fragPos;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragWorldNormal;

in mat4 fragModel;
in mat3 TBN;

layout (location=0) out vec4 finalColor;

uniform sampler2D colorTex;
uniform sampler2D specularTex;
uniform sampler2D normalTex;

uniform vec3 cameraPosition;

uniform vec3 diffuseColor = vec3(1.0, 1.0, 1.0);
uniform vec3 specularColor = vec3(1.0, 1.0, 1.0);
uniform float specularExponent = 32.0;
uniform float specularThreshold = 0.05f;

const int MAX_LIGHTS = 16;
uniform vec3  lightData[MAX_LIGHTS]; // Position or direction depending on type
uniform bool  lightType[MAX_LIGHTS]; // True: Direction, False: Point
uniform vec3  lightColor[MAX_LIGHTS];
uniform float lightIntensity[MAX_LIGHTS];
uniform vec3 ambientLightColor;

uniform vec2 resolution;

uniform float normalIntensity = 1.0;

vec3 finalNormal;

float Lambert(vec3 normal, vec3 lightDir)
{
	float NdotL = max(0.0, dot(normal, -lightDir));
	return pow(NdotL * 0.5 + 0.5, 2.0) * 2 - 0.5;
}

vec3 calcSpecular()
{
	float clampedSpecThreshold = clamp(specularThreshold, 0.0, 1.0);

	vec3 spec = vec3(0.0);
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		vec3 lightDir = lightData[i];
		if (!lightType[i]) // Check if point
		{
			lightDir = fragPos - lightData[i];
		} 

		vec3 viewDir = normalize(fragPos - cameraPosition);
		vec3 reflectDir = reflect(-normalize(lightDir), finalNormal);  
		spec += lightIntensity[i] * lightColor[i] * pow(max((dot(viewDir, reflectDir) - clampedSpecThreshold) / (1 - clampedSpecThreshold), 0.0), specularExponent);
	}

	return spec * texture(specularTex, fragTexCoord).rgb * specularColor;
}

vec3 calcDiffuse()
{
	vec3 lightCol = vec3(0.0);
	
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (lightType[i]) // Direction
		{
			lightCol += Lambert(finalNormal, normalize(lightData[i])) * lightIntensity[i] * lightColor[i];
		}
		else // Point
		{
			lightCol += Lambert(finalNormal, normalize(fragPos - lightData[i])) * lightIntensity[i] * lightColor[i];
		}
	}
	
	vec4 textureColor = texture(colorTex, fragTexCoord);
	return textureColor.rgb * max(lightCol, ambientLightColor) * diffuseColor;
}

vec3 calcNormal()
{
	return (TBN * normalize(mix(vec3(0.0, 0.0, 1.0), texture(normalTex, fragTexCoord).rgb * 2.0 - 1.0, clamp(normalIntensity, 0.0, 1.0))));
}

void main() {
	finalNormal = calcNormal();

	finalColor = vec4(calcDiffuse() + calcSpecular(), texture(colorTex, fragTexCoord).a);
}