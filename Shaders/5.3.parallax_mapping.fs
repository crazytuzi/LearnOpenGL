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
    /* number of depth layers */
    const float minLayers = 8.f;

    const float maxLayers = 32.f;

    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.f, 0.f, 1.f), viewDir)));

    /* calculate the size of each layer */
    float layerDepth = 1.f / numLayers;

    /* depth of current layer */
    float currentLayerDepth = 0.f;

    /* the amount to shift the texture coordinates per layer (from vector P) */
    vec2 P = viewDir.xy / viewDir.z * heightScale;

    vec2 deltaTexCoords = P / numLayers;

    /* get initial values */
    vec2 currentTexCoords = texCoords;

    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue)
    {
        /* shift texture coordinates along direction of P */
        currentTexCoords -= deltaTexCoords;

        /* get depthmap value at current texture coordinates */
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;

        /* get depth of next layer */
        currentLayerDepth += layerDepth;
    }

    /* get texture coordinates before collision (reverse operations) */
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    /* get depth after and before collision for linear interpolation */
    float afterDepth = currentDepthMapValue - currentLayerDepth;

    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    /* interpolation of texture coordinates */
    float weight = afterDepth / (afterDepth - beforeDepth);

    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.f - weight);

    return finalTexCoords;
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