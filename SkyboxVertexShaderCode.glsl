#version 430  // GLSL version your computer supports

in layout(location = 0) vec3 position;

uniform mat4 M;
uniform mat4 view;

out vec3 vertexFrag;

void main()
{
	vec4 newPosition = vec4(position, 1);
	gl_Position =  M * view * newPosition;
	vertexFrag = position;
}