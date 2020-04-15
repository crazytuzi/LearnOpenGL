#version 330 core

layout (triangles) in;

layout (triangle_strip, max_vertices = 18) out;

uniform mat4 shadowMatrices[6];

/* FragPos from GS (output per emitvertex) */
out vec4 FragPos;

void main()
{
	for(int face = 0; face < 6; ++face)
	{
		/* built-in variable that specifies to which face we render. */
		gl_Layer = face;

		/* for each triangle's vertices */
		for(int i = 0; i < 3; ++i)
		{
			FragPos = gl_in[i].gl_Position;

			gl_Position = shadowMatrices[face] * FragPos;

			EmitVertex();
		}

		EndPrimitive();
	}
}