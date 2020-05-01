#version 330 core

out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359f;

void main()
{
	/* The world vector acts as the normal of a tangent surface */
	/* from the origin, aligned to WorldPos. Given this normal, calculate all */
	/* incoming radiance of the environment. The result of this radiance */
	/* is the radiance of light coming from -Normal direction, which is what */
	/* we use in the PBR shader to sample irradiance. */
	vec3 N = normalize(WorldPos);
	
	vec3 irradiance = vec3(0.f);
	
	/* tangent space calculation from origin point */
	vec3 up = vec3(0.f, 1.f, 0.f);
	
	vec3 right = cross(up, N);
	
	up = cross(N, right);
	
	float sampleDelta = 0.025f;
	
	float nrSamples = 0.f;
	
	for (float phi = 0.f; phi < 2.f * PI; phi += sampleDelta)
	{
		for (float theta = 0.f; theta < 0.5f * PI; theta += sampleDelta)
		{
			/* spherical to cartesian (in tangent space) */
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			
			/* tangent space to world */
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
			
			irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
			
			nrSamples++;
		}
	}
	
	irradiance = PI * irradiance * (1.f / float(nrSamples));
	
	FragColor = vec4(irradiance, 1.f);
}
