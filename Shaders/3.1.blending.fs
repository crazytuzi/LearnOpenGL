#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture0;

void main()
{
    vec4 txeColor = texture(texture0, TexCoords);

    if(txeColor.a < 0.1f)
    {
        discard;
    }

    FragColor = txeColor;
}