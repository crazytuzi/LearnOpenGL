#version 330 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;

    sampler2D specular;

    float shininess;
};

struct Light {
    vec3 position;

    vec3 direction;

    float cutoff;

    float outerCutOff;

    vec3 ambient;

    vec3 diffuse;

    vec3 specular;

    float constant;

    float linear;

    float quadratic;
};

in vec2 TexCoords;

in vec3 FragPos;

in vec3 Normal;

uniform vec3 viewPos;

uniform Material material;

uniform Light light;

void main()
{
    /* ambient */
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    /* diffuse */
    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(light.position - FragPos);

    float diff = max(dot(norm, lightDir), 0.f);

    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;

    /* specular */
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 reflectDir = reflect(-lightDir, norm);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess);
    
    vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb;

    /* spotlight (soft edges) */
    float theta = dot(lightDir, normalize(-light.direction));

    float epsilon = light.cutoff - light.outerCutOff;

    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.f, 1.f);

    diffuse *= intensity;

    specular *= intensity;

    /* attenuation */
    float distance = length(light.position - FragPos);

    float attenuation = 1.f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;

    diffuse *= attenuation;

    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}