#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;

    vec3 Normal;

    vec2 TexCoords;
} fs_in;

uniform sampler2D floorTexture;

uniform vec3 lightPositions[4];

uniform vec3 lightColors[4];

uniform vec3 viewPos;

uniform bool gamma;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    /* diffuse */
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(lightDir, normal), 0.f);

    vec3 diffuse = diff * lightColor;

    /* specular */
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 reflectDir = reflect(-lightDir, normal);

    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(normal, halfwayDir), 0.f), 64.f);

    vec3 specular = spec * lightColor;

    /* simple attenuation */
    float distance = length(lightPos - fragPos);

    float attenuation = 1.f / (gamma ? distance * distance : distance);
    
    diffuse *= attenuation;

    specular *= attenuation;

    return diffuse + specular;
}

void main()
{
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;

    vec3 lighting = vec3(0.f);

    for(int i = 0; i < 4 ; ++i)
    {
        lighting += BlinnPhong(normalize(fs_in.Normal), fs_in.FragPos, lightPositions[i], lightColors[i]);
    }

    color *= lighting;

    if(gamma)
    {
        color = pow(color, vec3(1.f / 2.2f));
    }

    FragColor = vec4(color, 1.f);
}