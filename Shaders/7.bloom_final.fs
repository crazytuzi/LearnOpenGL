#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;

uniform sampler2D bloomBlur;

uniform bool bloom;

uniform float exposure;

void main()
{
	const float gamma = 2.2f;

	vec3 hdrColor = texture(scene, TexCoords).rgb;

	vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;

	if(bloom)
	{
		/* additive blending */
		hdrColor += bloomColor;
	}

	/* tone mapping */
	vec3 result = vec3(1.f) - exp(-hdrColor * exposure);

	/* also gamma correct while we're at it */
	result = pow(result, vec3(1.0 / gamma));

	FragColor = vec4(result, 1.f);
}