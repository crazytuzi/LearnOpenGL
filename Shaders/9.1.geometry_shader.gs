#version 330 core

layout (points) in;

layout (triangle_strip, max_vertices = 5) out;

in VS_OUT {
	vec3 color;
} gs_in[];

out vec3 fColor;

void build_house(vec4 position)
{
	/* gs_in[0] since there's only one input vertex */
	fColor = gs_in[0].color;

	/* 1:bottom-left */
	gl_Position = position + vec4(-0.2f, -0.2f, 0.f, 0.f);

	EmitVertex();

	/* 2:bottom-right */
	gl_Position = position + vec4(0.2f, -0.2f, 0.f, 0.f);

	EmitVertex();

	/* 3:top-left */
	gl_Position = position + vec4(-0.2f, 0.2f, 0.f, 0.f);

	EmitVertex();

	/* 4:top-right */
	gl_Position = position + vec4(0.2f, 0.2f, 0.f, 0.f);

	EmitVertex();

	/* 5:top */
	gl_Position = position + vec4(0.f, 0.4f, 0.f, 0.f);

	fColor = vec3(1.f, 1.f, 1.f);

	EmitVertex();

	EndPrimitive();
}

void main()
{
	build_house(gl_in[0].gl_Position);
}