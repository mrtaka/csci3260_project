#version 430

in layout(location=0) vec3 position;
in layout(location=1) vec2 textureCoordinate;
in layout(location=2) vec3 normalVector;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

void main()
{
	vec4 v = vec4(position, 1.0);
	vec4 newPosition = model * v;
	vec4 out_position = projection * view * model * v;
	gl_Position = out_position;

	vec4 normal_temp = model * vec4(normalVector, 0);
	normalWorld = normal_temp.xyz;

	vertexPositionWorld = newPosition.xyz;

	UV = textureCoordinate;
}