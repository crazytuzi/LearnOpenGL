#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;

    vec3 Normal;

    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;

uniform vec3 lightPos;

uniform vec3 viewPos;

uniform bool blinn;

void main()
{
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;

    /* ambient */
    vec3 ambient = 0.05f * color;

    /* diffuse */
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);

    vec3 normal = normalize(fs_in.Normal);

    float diff = max(dot(lightDir, normal), 0.f);

    vec3 diffuse = diff * color;

    /* specular */
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = 0.f;

    if(blinn)
    {
        vec3 halfwayDir = normalize(lightDir + viewDir);

        spec = pow(max(dot(normal, halfwayDir), 0.f), 32.f);
    }
    else
    {
        vec3 reflectDir = reflect(-lightDir, normal);

        spec = pow(max(dot(viewDir, reflectDir), 0.f), 8.f);
    }

    /* assuming bright white light color */
    vec3 specular = vec3(0.3f) * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.f);
}