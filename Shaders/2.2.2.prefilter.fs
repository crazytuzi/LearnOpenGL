#version 330 core

out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube environmentMap;

uniform float roughness;

const float PI = 3.14159265359f;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	
	float a2 = a*a;
	
	float NdotH = max(dot(N, H), 0.f);
	
	float NdotH2 = NdotH*NdotH;
	
	float nom   = a2;
	
	float denom = (NdotH2 * (a2 - 1.f) + 1.f);
	
	denom = PI * denom * denom;
	
	return nom / denom;
}

/* http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html */
/* efficient VanDerCorpus calculation. */
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	
	/* / 0x100000000 */
	return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.f * PI * Xi.x;
	
	float cosTheta = sqrt((1.f - Xi.y) / (1.f + (a*a - 1.f) * Xi.y));
	
	float sinTheta = sqrt(1.f - cosTheta*cosTheta);
	
	/* from spherical coordinates to cartesian coordinates - halfway vector */
	vec3 H;
	
	H.x = cos(phi) * sinTheta;
	
	H.y = sin(phi) * sinTheta;
	
	H.z = cosTheta;
	
	/* from tangent-space H vector to world-space sample vector */
	vec3 up = abs(N.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(1.f, 0.f, 0.f);
	
	vec3 tangent = normalize(cross(up, N));
	
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	
	return normalize(sampleVec);
}

void main()
{
	vec3 N = normalize(WorldPos);
	
	/* make the simplyfying assumption that V equals R equals the normal */
	vec3 R = N;
	
	vec3 V = R;
	
	const uint SAMPLE_COUNT = 1024u;
	
	vec3 prefilteredColor = vec3(0.f);
	
	float totalWeight = 0.f;
	
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		/* generates a sample vector that's biased towards the preferred alignment direction (importance sampling). */
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		
		vec3 L  = normalize(2.f * dot(V, H) * H - V);
		
		float NdotL = max(dot(N, L), 0.f);
		
		if (NdotL > 0.f)
		{
			/* sample from the environment's mip level based on roughness/pdf */
			float D = DistributionGGX(N, H, roughness);
			
			float NdotH = max(dot(N, H), 0.f);
			
			float HdotV = max(dot(H, V), 0.f);
			
			float pdf = D * NdotH / (4.f * HdotV) + 0.0001f;
			
			/* resolution of source cubemap (per face) */
			float resolution = 512.f;
			
			float saTexel = 4.f * PI / (6.f * resolution * resolution);
			
			float saSample = 1.f / (float(SAMPLE_COUNT) * pdf + 0.0001f);
			
			float mipLevel = roughness == 0.f ? 0.f : 0.5f * log2(saSample / saTexel);
			
			prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
			
			totalWeight += NdotL;
		}
	}
	
	prefilteredColor = prefilteredColor / totalWeight;
	
	FragColor = vec4(prefilteredColor, 1.f);
}
