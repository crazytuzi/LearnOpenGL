#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;

uniform sampler2D gNormal;

uniform sampler2D gAlbedo;

uniform sampler2D ssao;

struct Light {
    vec3 Position;

    vec3 Color;
    
    float Linear;

    float Quadratic;
};
uniform Light light;

void main()
{
    /* retrieve data from gbuffer */
    vec3 FragPos = texture(gPosition, TexCoords).rgb;

    vec3 Normal = texture(gNormal, TexCoords).rgb;

    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;

    float AmbientOcclusion = texture(ssao, TexCoords).r;

    /* then calculate lighting as usual */
    vec3 ambient = vec3(0.3f * Diffuse * AmbientOcclusion);

    vec3 lighting = ambient;

    /* viewpos is (0.0.0) */
    vec3 viewDir = normalize(-FragPos);

    /* diffuse */
    vec3 lightDir = normalize(light.Position - FragPos);

    vec3 diffuse = max(dot(Normal, lightDir), 0.f) * Diffuse * light.Color;

    /* specular */
    vec3 halfwayDir = normalize(lightDir  + viewDir);

    float spec = pow(max(dot(Normal, halfwayDir), 0.f), 8.f);

    vec3 specular = light.Color * spec;

    /* attenuation */
    float distance = length(light.Position - FragPos);

    float attenuation = 1.f / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);

    diffuse *= attenuation;

    specular *= attenuation;

    lighting += diffuse + specular;

    FragColor = vec4(lighting, 1.f);
}