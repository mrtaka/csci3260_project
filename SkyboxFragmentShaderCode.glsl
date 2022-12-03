#version 430 // GLSL version your computer supports

in vec3 vertexFrag;

uniform samplerCube skybox;

out vec4 colorFrag;

void main()
{
	colorFrag = texture(skybox, vertexFrag);
}
