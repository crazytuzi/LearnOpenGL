#version 330 core

layout (location = 0) out vec4 FragColor;

layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 FragPos;

    vec3 Normal;

    vec2 TexCoords;
} fs_in;

uniform vec3 lightColor;

void main()
{
    FragColor = vec4(lightColor, 1.f);

    float brightness = dot(FragColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));

    if(brightness > 1.f)
    {
        BrightColor = vec4(FragColor.rgb, 1.f);
    }
    else
    {
        BrightColor = vec4(0.f, 0.f, 0.f, 1.f);
    }
}