#version 330 core

layout (triangles) in;

layout (line_strip, max_vertices = 6) out;

in VS_OUT {
	vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.01f;

void GenerateLine(int index)
{
	gl_Position = gl_in[index].gl_Position;

	EmitVertex();

	gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.f) * MAGNITUDE;

	EmitVertex();

	EndPrimitive();
}

void main()
{
	/* first vertex normal */
	GenerateLine(0);

	/* second vertex normal */
	GenerateLine(1);

	/* third vertex normal */
	GenerateLine(2);
}