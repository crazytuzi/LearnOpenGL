#version 330 core

out vec4 FragColor;

float near = 0.1f;

float far = 100.f;

float LinearizeDepth(float depth)
{
    /* back to NDC */
    float z = depth * 2.f - 1.f;

    return (2.f * near * far) / (far + near -z * (far - near));
}

void main()
{   
    /* divide by far for demonstration */
    float depth = LinearizeDepth(gl_FragCoord.z) / far;

    FragColor = vec4(vec3(depth), 1.f);
}