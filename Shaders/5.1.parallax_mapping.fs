#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;

    vec2 TexCoords;

    vec3 TangentLightPos;

    vec3 TangentViewPos;

    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;

uniform sampler2D normalMap;

uniform sampler2D depthMap;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height = texture(depthMap, texCoords).r;

    return texCoords - viewDir.xy * (height * heightScale);
}

void main()
{
    /* offset texture coordinates with Parallax Mapping */
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

    vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);

    if(texCoords.x > 1.f || texCoords.y > 1.f || texCoords.x < 0.f || texCoords.y < 0.f)
    {
        discard;
    }

    /* obtain normal from normal map in range [0,1] */
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;

    /* transform normal vector to range [-1,1] */
    /* this normal is in tangent space */
    normal = normalize(normal * 2.f - 1.f);

    /* get diffuse color */
    vec3 color = texture(diffuseMap, fs_in.TexCoords).rgb;

    // ambient
    vec3 ambient = 0.1 * color;

    /* diffuse */
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);

    float diff = max(dot(lightDir, normal), 0.0);

    vec3 diffuse = diff * color;

    /* specular */
    vec3 reflectDir = reflect(-lightDir, normal);

    vec3 halfwayDir = normalize(lightDir + viewDir);  

    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}