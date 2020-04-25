#version 330 core

out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;

uniform sampler2D gNormal;

uniform sampler2D texNoise;

uniform vec3 samples[64];

/* parameters (you'd probably want to use them as uniforms to more easily tweak the effect) */
int kernelSize = 64;

float radius = 0.5f;

float bias = 0.025f;

/* tile noise texture over screen based on screen dimensions divided by noise size */
const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0);

uniform mat4 projection;

void main()
{
	/* get input for SSAO algorithm */
	vec3 fragPos = texture(gPosition, TexCoords).xyz;

    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);

    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

	/* create TBN change-of-basis matrix: from tangent-space to view-space */
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));

    vec3 bitangent = cross(normal, tangent);

    mat3 TBN = mat3(tangent, bitangent, normal);

	/* iterate over the sample kernel and calculate occlusion factor */
	float occlusion = 0.f;

	for(int i = 0; i < kernelSize; ++i)
    {
        /* get sample position */
        /* from tangent to view-space */
        vec3 sample = TBN * samples[i];

        sample = fragPos + sample * radius; 
        
        /* project sample position (to sample texture) (to get position on screen/texture) */
        vec4 offset = vec4(sample, 1.0);

        /* from view to clip-space */
        offset = projection * offset;

        /* perspective divide */
        offset.xyz /= offset.w;

        /* transform to range 0.0 - 1.0 */
        offset.xyz = offset.xyz * 0.5 + 0.5;
        
        /* get sample depth */
        /* get depth value of kernel sample */
        float sampleDepth = texture(gPosition, offset.xy).z;
        
        /* range check & accumulate */
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));

        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }

    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = occlusion;
}