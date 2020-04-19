#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

uniform bool horizontal;

uniform float weight[5] = float[] (0.2270270270f, 0.1945945946f, 0.1216216216f, 0.0540540541f, 0.0162162162f);

void main()
{
	/* gets size of single texel */
	vec2 tex_offset = 1.f / textureSize(image, 0);

	vec3 result = texture(image, TexCoords).rgb * weight[0];

	if(horizontal)
	{
		for(int i = 1; i < 5; ++i)
		{
			result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.f)).rgb * weight[i];

			result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.f)).rgb * weight[i];
		}
	}
	else
	{
		for(int i = 1; i < 5; ++i)
		{
			result += texture(image, TexCoords + vec2(0.f, tex_offset.y * i)).rgb * weight[i];

			result += texture(image, TexCoords - vec2(0.f, tex_offset.y * i)).rgb * weight[i];
		}
	}

	FragColor = vec4(result, 1.f);
}
