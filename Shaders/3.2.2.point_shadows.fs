#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;

    vec3 Normal;

    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;

uniform samplerCube depthMap;

uniform vec3 lightPos;

uniform vec3 viewPos;

uniform float far_plane;

/* array of offset direction for sampling */
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 fragPos)
{
    /* get vector between fragment position and light position */
    vec3 fragToLight = fragPos - lightPos;

    /* now get current linear depth as the length between the fragment and light position */
    float currentDepth = length(fragToLight);

    float shadow = 0.f;

    float bias = 0.05f;

    int samples = 20;

    float viewDistance = length(viewPos - fragPos);

    float diskRadius = (1.f + (viewDistance / far_plane)) / 25.f;

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;

        /* undo mapping [0;1] */
        closestDepth *= far_plane;

        if(currentDepth - bias > closestDepth)
        {
            shadow += 1.f;
        }
    }

    shadow /= float(samples);

    return shadow;
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;

    vec3 normal = normalize(fs_in.Normal);

    vec3 lightColor = vec3(0.3);

    /* ambient */
    vec3 ambient = 0.3 * color;

    /* diffuse */
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);

    float diff = max(dot(lightDir, normal), 0.0);

    vec3 diffuse = diff * lightColor;

    /* specular */
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 reflectDir = reflect(-lightDir, normal);

    float spec = 0.0;

    vec3 halfwayDir = normalize(lightDir + viewDir);  

    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);

    vec3 specular = spec * lightColor;  
	
	/* calculate shadow */
	float shadow = ShadowCalculation(fs_in.FragPos);

	vec3 lighting = (ambient + (1.f - shadow) * (diffuse + specular)) * color;

	FragColor = vec4(lighting, 1.f);
}