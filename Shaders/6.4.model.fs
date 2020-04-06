#version 330 core

out vec4 FragColor;

in vec3 Normal;

in vec3 Position;

in vec2 TexCoords;

uniform vec3 cameraPos;

uniform sampler2D texture_diffuse0;

uniform sampler2D texture_reflection0;

uniform samplerCube skybox;

void main()
{    
    /* Diffuse */
    vec3 diffuse = vec3(texture(texture_diffuse0, TexCoords));

    /* Reflection */
    vec3 I = normalize(Position - cameraPos);

    vec3 R = reflect(I, normalize(Normal));

    float reflect_intensity = texture(texture_reflection0, TexCoords).r;

    vec3 reflection;

    /* Only sample reflections when above a certain treshold */
    if(reflect_intensity > 0.1)
    {
        reflection = vec3(texture(skybox, R)) * reflect_intensity;
    }
    
    /* Combine */ 
    FragColor = vec4(diffuse + reflection, 1.f);
}