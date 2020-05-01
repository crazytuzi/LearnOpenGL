#version 330 core

out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{
	vec3 envColor = textureLod(environmentMap, WorldPos, 0.f).rgb;
	
	/* HDR tonemap and gamma correct */
	envColor = envColor / (envColor + vec3(1.f));
	
	envColor = pow(envColor, vec3(1.f/2.2f));
	
	FragColor = vec4(envColor, 1.f);
}
