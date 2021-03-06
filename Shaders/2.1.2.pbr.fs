#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

in vec3 WorldPos;

in vec3 Normal;

/* material parameters */
uniform vec3 albedo;

uniform float metallic;

uniform float roughness;

uniform float ao;

/* IBL */
uniform samplerCube irradianceMap;

/* lights */
uniform vec3 lightPositions[4];

uniform vec3 lightColors[4];

uniform vec3 cameraPos;

const float PI = 3.14159265359f;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	
	float a2 = a * a;
	
	float NdotH = max(dot(N, H), 0.f);
	
	float NdotH2 = NdotH * NdotH;
	
	float nom   = a2;
	
	float denom = (NdotH2 * (a2 - 1.f) + 1.f);
	
	denom = PI * denom * denom;
	
	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.f);
	
	float k = (r*r) / 8.f;
	
	float nom   = NdotV;
	
	float denom = NdotV * (1.f - k) + k;
	
	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.f);
	
	float NdotL = max(dot(N, L), 0.f);
	
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.f - F0) * pow(1.f - cosTheta, 5.f);
}

void main()
{
	vec3 N = Normal;
	
	vec3 V = normalize(cameraPos - WorldPos);
	
	vec3 R = reflect(-V, N);
	
	/* calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 */
	/* of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow) */
	vec3 F0 = vec3(0.04f);
	
	F0 = mix(F0, albedo, metallic);
	
	/* reflectance equation */
	vec3 Lo = vec3(0.f);
	
	for (int i = 0; i < 4; ++i)
	{
		/* calculate per-light radiance */
		vec3 L = normalize(lightPositions[i] - WorldPos);
		
		vec3 H = normalize(V + L);
		
		float distance = length(lightPositions[i] - WorldPos);
		
		float attenuation = 1.f / (distance * distance);
		
		vec3 radiance = lightColors[i] * attenuation;
		
		/* Cook-Torrance BRDF */
		float NDF = DistributionGGX(N, H, roughness);
		
		float G   = GeometrySmith(N, V, L, roughness);
		
		vec3 F    = fresnelSchlick(max(dot(H, V), 0.f), F0);
		
		vec3 nominator = NDF * G * F;
		
		/* 0.001 to prevent divide by zero. */
		float denominator = 4 * max(dot(N, V), 0.f) * max(dot(N, L), 0.f) + 0.001f;
		
		vec3 specular = nominator / denominator;
		
		/* kS is equal to Fresnel */
		vec3 kS = F;
		
		/* for energy conservation, the diffuse and specular light can't */
		/* be above 1.0 (unless the surface emits light); to preserve this */
		/* relationship the diffuse component (kD) should equal 1.0 - kS. */
		vec3 kD = vec3(1.f) - kS;
		
		/* multiply kD by the inverse metalness such that only non-metals */
		/* have diffuse lighting, or a linear blend if partly metal (pure metals */
		/* have no diffuse light). */
		kD *= 1.f - metallic;
		
		/* scale light by NdotL */
		float NdotL = max(dot(N, L), 0.f);
		
		/* add to outgoing radiance Lo */
		/* note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again */
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	
	/* ambient lighting (we now use IBL as the ambient term) */
	vec3 kS = fresnelSchlick(max(dot(N, V), 0.f), F0);
	
	vec3 kD = 1.f - kS;
	
	kD *= 1.f - metallic;
	
	vec3 irradiance = texture(irradianceMap, N).rgb;
	
	vec3 diffuse = irradiance * albedo;
	
	vec3 ambient = (kD * diffuse) * ao;
	
	vec3 color = ambient + Lo;
	
	/* HDR tonemapping */
	color = color / (color + vec3(1.f));
	
	/* gamma correct */
	color = pow(color, vec3(1.f/2.2f));
	
	FragColor = vec4(color, 1.f);
}
